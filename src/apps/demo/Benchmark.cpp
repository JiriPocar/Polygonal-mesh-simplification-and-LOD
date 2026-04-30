/**
 * @author Jiri Pocarovsky (xpocar01@stud.fit.vutbr.cz)
 * @file Benchmark.hpp
 * @brief Benchmark class for performance testing of the Spiral application.
 */

#include "Benchmark.hpp"
#include <iostream>
#include <filesystem>

Benchmark::Benchmark()
{
	
}

void Benchmark::startStatic()
{
	csvFile.open("plot/spiralBenchmark.csv");
	if (!csvFile.is_open())
	{
		std::cerr << "Failed to create benchmark CSV file!" << std::endl;
		return;
	}

	csvFile << "Instances,GPULOD,GPUSpiral,EnableLOD,SceneFaces,AvgFrameTimeMs\n";

	running = true;
	method = BenchmarkMethod::STATIC_CAMERA;
	currentConfigIdx = 0;
	maxInstanceCount = 100000;
	applyConfigFlag = true;
	inWarmup = true;
	currentTimer = 0.0f;
}

void Benchmark::startDynamic()
{
	csvFile.open("plot/spiralBenchmarkMoving.csv");
	if (!csvFile.is_open())
	{
		std::cerr << "Failed to create benchmark CSV file!" << std::endl;
		return;
	}

	csvFile << "Instances,CAMZ,SceneFaces,LOD0,LOD1,LOD2,LOD3\n";

	running = true;
	method = BenchmarkMethod::MOVING_CAMERA;
	inWarmup = true;
	currentTimer = 0.0f;
	applyConfigFlag = true;
}

void Benchmark::calibrate(float deltaTime)
{
	if (currentTimer > 1.0f && currentTimer <= CALIBRATION_TIME)
	{
		// record frame times
		currentConfigDataStatic.framesMeasured++;
		currentConfigDataStatic.cumulatedTime += (deltaTime * 1000.0f);
	}
	else if (currentTimer > CALIBRATION_TIME)
	{
		if (calibrationStage == 1)
		{
			avgTime1 = currentConfigDataStatic.cumulatedTime / currentConfigDataStatic.framesMeasured;
			calibrationStage = 2;
		}
		else if (calibrationStage == 3)
		{
			avgTime2 = currentConfigDataStatic.cumulatedTime / currentConfigDataStatic.framesMeasured;

			// y = ax + b
			// a = (y2 - y1) / (x2 - x1)
			float a = (avgTime2 - avgTime1) / (500000.0f - 100000.0f);
			// b = y - ax
			float b = avgTime1 - a * 100000.0f;
			// x = (y - b) / a  for x = 30fps (33.333ms)
			float estimatedMax = (33.333f - b) / a;

			// calculate step size
			// get rough step size by dividing estimated max by target number of steps
			uint32_t roughStepSize = static_cast<uint32_t>(estimatedMax / TARGETSTEPNUMBER);
			// round to nearest power of 10
			uint32_t magnitude = static_cast<uint32_t>(pow(10, floor(log10(roughStepSize))));
			// round to nearest multiple of magnitude
			stepSize = ((roughStepSize + magnitude / 2) / magnitude) * magnitude;
			// minimum step should be about 10k isntances
			stepSize = std::max(stepSize, 10000u);

			// set max instances and cap to 100mil if needed
			maxInstanceCount = static_cast<uint32_t>(estimatedMax);
			if (maxInstanceCount > 1e8) maxInstanceCount = 1e8;

			isCalibrated = true;
		}
	}
}

void Benchmark::update(float deltaTime, SpiralScene& spiralScene, SpiralRenderer& renderer, glm::vec3& camPos)
{
	if (!isRunning())
	{
		return;
	}

	currentTimer += deltaTime;

	if (method == BenchmarkMethod::STATIC_CAMERA)
	{
		if (!isCalibrated)
		{
			if (calibrationStage == 0)
			{
				// setup the scene for the first calibration step with 100k instances
				spiralScene.config.instanceCount = 100000;
				spiralScene.reallocBuffers(100000);
				renderer.refreshComputeDescriptors();
				spiralScene.updateSpiralPositions(0.0f, false);

				currentTimer = 0.0f;
				currentConfigDataStatic.cumulatedTime = 0.0f;
				currentConfigDataStatic.framesMeasured = 0;
				calibrationStage = 1;
			}
			else if (calibrationStage == 1)
			{
				// do the actual calibration
				calibrate(deltaTime);
			}
			else if (calibrationStage == 2)
			{
				// setup the scene for the second calibration step with 500k instances
				spiralScene.config.instanceCount = 500000;
				spiralScene.reallocBuffers(500000);
				renderer.refreshComputeDescriptors();
				spiralScene.updateSpiralPositions(0.0f, false);

				currentTimer = 0.0f;
				currentConfigDataStatic.cumulatedTime = 0.0f;
				currentConfigDataStatic.framesMeasured = 0;
				calibrationStage = 3;
			}
			else if (calibrationStage == 3)
			{
				// do the actual calibration
				calibrate(deltaTime);
			}

			// setup configuration properties and scene after calibration
			if (isCalibrated)
			{
				spiralScene.reallocBuffers(maxInstanceCount);
				renderer.refreshComputeDescriptors();

				buildConfigs();

				currentConfigIdx = 0;
				inWarmup = true;
				currentTimer = 0.0f;
				applyConfigFlag = true;
			}
			return;
		}

		if (inWarmup)
		{
			// wait for th warmup time to pass
			if (currentTimer >= WARMUPTIME)
			{
				inWarmup = false;

				// reset the timer and data for the actual benchmark
				currentTimer = 0.0f;
				currentConfigDataStatic.cumulatedTime = 0.0f;
				currentConfigDataStatic.framesMeasured = 0;
				currentConfigDataStatic.drawnTriangles = spiralScene.calculateCurrentDrawnTriangles(camPos, currentConfigDataStatic.lodCounts);
			}
		}
		else
		{
			// update benchmark data
			currentConfigDataStatic.framesMeasured++;
			currentConfigDataStatic.cumulatedTime += (deltaTime * 1000);

			// end if above the benchmark time
			if (currentTimer >= BENCHMARK_CONFIG_TIME)
			{
				float avgMs = currentConfigDataStatic.cumulatedTime / currentConfigDataStatic.framesMeasured;
				auto config = configs[currentConfigIdx];
				saveToCSV();
				goNextConfig();
			}
		}
	}
	else if (method == BenchmarkMethod::MOVING_CAMERA)
	{
		// move camera
		camPos.z += 150.0f * deltaTime;

		if (inWarmup)
		{
			if (currentTimer >= 2.0f)
			{
				inWarmup = false;
				currentTimer = 0.0f;
			}
		}
		else
		{
			currentConfigDataDynamic.camZ = camPos.z;

			if (camPos.z - currentConfigDataDynamic.lastCamZ >= 20.0f)
			{
				// get number of drawn triangles and LOD distribution for current camera position
				currentConfigDataDynamic.drawnTriangles = spiralScene.calculateCurrentDrawnTriangles(camPos, currentConfigDataDynamic.lodCounts);

				// Instances,CAMZ,SceneFaces,LOD0,LOD1,LOD2,LOD3\n";
				csvFile << spiralScene.config.instanceCount << ","
						<< camPos.z << ","
						<< currentConfigDataDynamic.drawnTriangles << ","
						<< currentConfigDataDynamic.lodCounts[0] << ","
						<< currentConfigDataDynamic.lodCounts[1] << ","
						<< currentConfigDataDynamic.lodCounts[2] << ","
						<< currentConfigDataDynamic.lodCounts[3] << "\n";

				// reset cummulated statistics
				currentConfigDataDynamic.lastCamZ = camPos.z;
			}

			if (camPos.z > 5100.0f)
			{
				running = false;
				csvFile.close();
			}
		}
	}
}

void Benchmark::buildConfigs()
{
	configs.clear();

	for (uint32_t count = stepSize; count <= maxInstanceCount; count += stepSize)
	{
		// enable LOD
		configs.push_back({ count, false, false, true }); // CPU LOD / CPU spiral
		configs.push_back({ count, true, false, true });  // GPU LOD / CPU spiral
		configs.push_back({ count, true, true, true });   // GPU LOD / GPU spiral

		// disable LOD
		configs.push_back({ count, false, false, false }); // CPU LOD / CPU spiral
		configs.push_back({ count, true, false, false });  // GPU LOD / CPU spiral
		configs.push_back({ count, true, true, false });   // GPU LOD / GPU spiral
	}
}

void Benchmark::goNextConfig()
{
	if (currentConfigIdx + 1 >= configs.size())
	{
		running = false;
		csvFile.close();
	}
	else
	{
		currentConfigIdx++;
		inWarmup = true;
		currentTimer = 0.0f;
		applyConfigFlag = true;
	}
}

void Benchmark::saveToCSV()
{
	float avgTime;
	if (currentConfigDataStatic.framesMeasured > 0)
	{
		avgTime = currentConfigDataStatic.cumulatedTime / currentConfigDataStatic.framesMeasured;
	}
	else
	{
		// shouldnt happen, though when it does just set -1 as error to be parsed later
		avgTime = -1.0f;
	}

	auto c = configs[currentConfigIdx];

	csvFile << c.instances << ","
			<< (c.useGPULOD ? "GPU" : "CPU") << ","
			<< (c.useGPUSpiral ? "GPU" : "CPU") << ","
			<< (c.enableLOD ? "Enabled" : "Disabled") << ","
			<< currentConfigDataStatic.drawnTriangles << ","
			<< avgTime << "\n";
}

/* End of the Benchmark.cpp file */
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
	buildConfigs();
}

void Benchmark::startStatic()
{
	// TODO(!): fix this hardcoded path
	csvFile.open("C:/Users/tf2ma/source/repos/Renderer/plot/spiralBenchmark.csv");
	if (!csvFile.is_open())
	{
		std::cerr << "Failed to create benchmark CSV file!" << std::endl;
		return;
	}

	csvFile << "Instances,GPULOD,GPUSpiral,EnableLOD,SceneFaces,AvgFrameTimeMs\n";

	running = true;
	method = BenchmarkMethod::STATIC_CAMERA;
	currentConfigIdx = 0;
	applyConfigFlag = true;
	inWarmup = true;
	currentTimer = 0.0f;
}

void Benchmark::startDynamic()
{
	csvFile.open("C:/Users/tf2ma/source/repos/Renderer/plot/spiralBenchmarkMoving.csv");
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

void Benchmark::update(float deltaTime, SpiralScene& spiralScene, glm::vec3& camPos)
{
	if (!isRunning())
	{
		return;
	}

	currentTimer += deltaTime;

	if (method == BenchmarkMethod::STATIC_CAMERA)
	{
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
	std::vector<uint32_t> instanceCounts = { 6000000, 5600000, 5200000, 4800000, 4400000, 4000000, 3600000, 3200000, 2800000, 2400000, 2000000, 1600000, 1200000, 800000, 400000 };

	for (uint32_t count : instanceCounts)
	{
		// enable LOD
		configs.push_back({ count, false, false, true }); // CPU LOD / CPU spiral
		configs.push_back({ count, true, false, true });  // GPU LOD / CPU spiral
		configs.push_back({ count, true, true, true });   // GPU LOD / GPU spiral

		// disable LOD
		configs.push_back({ count, false, false, false }); // CPU LOD / CPU spiral
		configs.push_back({ count, true, false, false });  // GPU LOD / CPU spiral
		configs.push_back({ count, true, true, false });   // GPU LOD / GPU spiral

		// TODO: add more configs (though I think this should do)
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
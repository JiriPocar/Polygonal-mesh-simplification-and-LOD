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

void Benchmark::start()
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
	currentConfigIdx = 0;
	applyConfigFlag = true;
	inWarmup = true;
	currentTimer = 0.0f;
}

void Benchmark::update(float deltaTime, SpiralScene& spiralScene, glm::vec3& camPos)
{
	if (!isRunning())
	{
		return;
	}

	currentTimer += deltaTime;

	if (inWarmup)
	{
		// wait for th warmup time to pass
		if (currentTimer >= WARMUPTIME)
		{
			inWarmup = false;

			// reset the timer and data for the actual benchmark
			currentTimer = 0.0f;
			currentConfigData.cumulatedTime = 0.0f;
			currentConfigData.framesMeasured = 0;
			currentConfigData.drawnTriangles = spiralScene.calculateCurrentDrawnTriangles(camPos);
		}
	}
	else
	{
		// update benchmark data
		currentConfigData.framesMeasured++;
		currentConfigData.cumulatedTime += (deltaTime * 1000);

		// end if above the benchmark time
		if (currentTimer >= BENCHMARK_CONFIG_TIME)
		{
			saveToCSV();
			goNextConfig();
		}
	}
}

void Benchmark::buildConfigs()
{
	std::vector<uint32_t> instanceCounts = { 200000, 400000, 600000, 800000, 1000000, 1200000, 1400000, 1600000, 1800000, 2000000 };

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
	if (currentConfigData.framesMeasured > 0)
	{
		avgTime = currentConfigData.cumulatedTime / currentConfigData.framesMeasured;
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
			<< currentConfigData.drawnTriangles << ","
			<< avgTime << "\n";
}

/* End of the Benchmark.cpp file */
/**
 * @author Jiri Pocarovsky (xpocar01@stud.fit.vutbr.cz)
 * @file Benchmark.hpp
 * @brief Benchmark class for performance testing of the Spiral application.
 */

#pragma once
#include "../common/scene/SpiralScene.hpp"
#include <vector>
#include <string>
#include <fstream>

const float WARMUPTIME = 2.0f;
const float BENCHMARK_CONFIG_TIME = 10.0f;

// tested parameteres
struct BenchmarkConfig {
	uint32_t instances;
	bool useGPULOD;
	bool useGPUSpiral;
	bool enableLOD;
};

// collected data
struct BenchmarkConfigData {
	uint32_t framesMeasured;
	float cumulatedTime;
	uint32_t drawnTriangles;
};

class Benchmark {
public:
	Benchmark();
	~Benchmark() { if (csvFile.is_open()) csvFile.close(); }

	/**
	* @brief Sets the benchmark process to start. Opens the CSV file and
	*		 sets the initial internal variables.
	*/
	void start();

	/**
	* @brief Controls the benchmark process, mesasures frame times and manages
	*		 the transition between different benchmark configurations.
	*  
	* @note Call each frame with the delta time.
	* 
	* @param deltaTime Time elapsed since the last frame in seconds.
	*/
	void update(float deltaTime, SpiralScene& spiralScene, glm::vec3& camPos);

	/**
	* @brief Transitions into the next config. If no more configs are left,
	*		 ends up the benchmark.
	*/
	void goNextConfig();

	// getters
	BenchmarkConfig getCurrentConfig() const { return configs[currentConfigIdx]; }
	uint32_t getCurrentConfigIndex() const { return currentConfigIdx; }
	bool isRunning() const { return running; }
	bool needsApplyConfig() const { return applyConfigFlag; }
	void clearApplyConfigFlag() { applyConfigFlag = false; }

private:
	/**
	 * @brief Builds the list of benchmark configurations to be tested and saves them
	 *		 in the internal vector.
	*/
	void buildConfigs();

	/**
	 * @brief Saves the benchmark results of the current configuration into the CSV file.
	*/
	void saveToCSV();

	std::ofstream csvFile;

	std::vector<BenchmarkConfig> configs;
	uint32_t currentConfigIdx;

	BenchmarkConfigData currentConfigData = { 0, 0.0f, 0 };
	float currentTimer = 0.0f;

	bool running = false;
	bool inWarmup = false;
	bool applyConfigFlag = false;
};

/* End of the Benchmark.hpp file */
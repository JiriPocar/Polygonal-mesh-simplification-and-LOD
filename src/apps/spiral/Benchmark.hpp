/**
 * @author Jiri Pocarovsky (xpocar01@stud.fit.vutbr.cz)
 * @file Benchmark.hpp
 * @brief Benchmark class for performance testing of the Spiral application.
 */

#pragma once
#include "SpiralScene.hpp"
#include "SpiralRenderer.hpp"
#include <vector>
#include <string>
#include <fstream>

const float WARMUPTIME = 2.0f;
const float CALIBRATION_TIME = 10.0f;
const float BENCHMARK_CONFIG_TIME = 5.0f;

const int TARGET_STEP_NUMBER = 15;

enum class BenchmarkMethod {
	STATIC_CAMERA,
	MOVING_CAMERA
};

// tested parameteres
struct BenchmarkConfig {
	uint32_t instances;
	bool useGPULOD;
	bool useGPUSpiral;
	bool enableLOD;
};

// collected data
struct BenchmarkConfigDataStatic {
	uint32_t framesMeasured;
	float cumulatedTime;
	uint64_t drawnTriangles;
	std::array<uint32_t, 4> lodCounts;
};

struct BenchmarkConfigDataDynamic {
	float camZ;
	float lastCamZ;
	uint64_t drawnTriangles;
	std::array<uint32_t, 4> lodCounts;
};

class Benchmark {
public:
	Benchmark();
	~Benchmark() = default;

	/**
	* @brief Sets the benchmark process to start. Opens the CSV file and
	*		 sets the initial internal variables.
	*/
	void startStatic();
	void startDynamic();

	/**
	* @brief Controls the benchmark process, mesasures frame times and manages
	*		 the transition between different benchmark configurations.
	*  
	* @note Call each frame with the delta time.
	* 
	* @param deltaTime Time elapsed since the last frame in seconds.
	*/
	void update(float deltaTime, SpiralScene& spiralScene, SpiralRenderer& renderer, glm::vec3& camPos);

	/**
	* @brief Transitions into the next config. If no more configs are left,
	*		 ends up the benchmark.
	*/
	void goNextConfig();

	// getters and setters
	BenchmarkConfig getCurrentConfig() const {
		if (configs.empty())
		{
			return { maxInstanceCount, true, true, true};
		}
		return configs[currentConfigIdx];
	}
	uint32_t getCurrentConfigIndex() const { return currentConfigIdx; }
	size_t getNumberOfConfigs() const { return configs.size(); }
	BenchmarkConfig getConfigAtIdx(int idx) const { return configs[idx]; }
	uint32_t getMaxInstanceCount() const { return maxInstanceCount; }
	bool getCalibrationStatus() const { return isCalibrated; }
	uint32_t getStepSize() const { return stepSize; }
	bool isRunning() const { return running; }
	bool needsApplyConfig() const { return applyConfigFlag; }
	void clearApplyConfigFlag() { applyConfigFlag = false; }
	BenchmarkMethod getMethod() const { return method; }
	void setMethod(BenchmarkMethod newMethod) { method = newMethod; }

private:
	/**
	 * @brief Builds the list of benchmark configurations to be tested and saves them
	 *		 in the internal vector.
	*/
	void buildConfigs();

	/**
	* @brief Calibrates the benchmark on the start. Sets max instance count and step size.
	* 
	* @param deltaTime Time elapsed since the last frame in seconds.
	*/
	void calibrate(float deltaTime);

	/**
	 * @brief Saves the benchmark results of the current configuration into the CSV file.
	*/
	void saveToCSV();

	std::ofstream csvFile;

	std::vector<BenchmarkConfig> configs;
	uint32_t currentConfigIdx = 0;

	BenchmarkConfigDataStatic currentConfigDataStatic = { 0, 0.0f, 0 };
	BenchmarkConfigDataDynamic currentConfigDataDynamic = { 0.0f, 0.0f, 0 };
	float currentTimer = 0.0f;
	uint32_t maxInstanceCount = 0;

	BenchmarkMethod method = BenchmarkMethod::STATIC_CAMERA;

	int calibrationStage = 0;
	float avgTime1 = 0.0f;
	float avgTime2 = 0.0f;

	bool running = false;
	bool inWarmup = false;
	bool applyConfigFlag = false;
	bool isCalibrated = false;
	uint32_t stepSize = 0;
};

/* End of the Benchmark.hpp file */
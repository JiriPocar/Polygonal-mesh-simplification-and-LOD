# Author: Jiri Pocarovsky
# File: plotLOD.py
# 
# This script plots graphs corresponding to the Spiral scene benchmark results.

import os
from turtle import color
import matplotlib.pyplot as plt
import pandas as pd
import numpy as np

def plotFrameTimeInstances(df):
    print("Plotting frame time to instances...")

    # GPU / CPU Compute plot
    dfLODON = df[df['EnableLOD'] == 'Enabled']
    cpuOnly = dfLODON[(dfLODON['GPULOD'] == 'CPU') & (dfLODON['GPUSpiral'] == 'CPU')]['AvgFrameTimeMs'].values
    gpuLODcpuSpiral = dfLODON[(dfLODON['GPULOD'] == 'GPU') & (dfLODON['GPUSpiral'] == 'CPU')]['AvgFrameTimeMs'].values
    gpuOnly = dfLODON[(dfLODON['GPULOD'] == 'GPU') & (dfLODON['GPUSpiral'] == 'GPU')]['AvgFrameTimeMs'].values

    # take each unique instance count
    xLabels = sorted(df['Instances'].unique())
    formattedLabels = [f"{int(x):,}".replace(',', ' ') for x in xLabels]
    
    # plot
    plt.figure(figsize=(12, 8))
    plt.grid(which='major', linestyle='--', color='gray', alpha=0.7)
    
    plt.plot(xLabels, cpuOnly, marker='o', linewidth=2, linestyle='-', label='CPU pozice na spirále, CPU výběr LOD')
    plt.plot(xLabels, gpuLODcpuSpiral, marker='v', linewidth=2, linestyle='-', label='CPU pozice na spirále, GPU výběr LOD')
    plt.plot(xLabels, gpuOnly, marker='s', linewidth=2, linestyle='-', label='GPU pozice na spirále, GPU výběr LOD')

    plt.title('Vliv výpočtů na grafické kartě (LOD zapnut)', pad=20)
    plt.ylabel('Čas vykreslení snímku [ms]', labelpad=10)
    plt.xlabel('Počet instancí modelu ve scéně', labelpad=10)

    plt.xticks(xLabels, formattedLabels, rotation=30)

    plt.legend()
    plt.tight_layout()
    plt.savefig('GPUINFLUENCE.svg')
    plt.close()
    print("Done...")
    

def plotLODEnablingGPU(df):
    print("Plotting FPS to instances with LOD Enabled / Disabled ...")

    # take each unique instance count
    xLabels = sorted(df['Instances'].unique())
    xPos = np.arange(len(xLabels))

    gpuOnly = df[(df['GPULOD'] == 'GPU') & (df['GPUSpiral'] == 'GPU')]
    LODON = gpuOnly[gpuOnly['EnableLOD'] == 'Enabled']['AvgFrameTimeMs'].values
    LODOFF = gpuOnly[gpuOnly['EnableLOD'] == 'Disabled']['AvgFrameTimeMs'].values

    # plot
    plt.figure(figsize=(12, 8))
    plt.grid(axis='y', which='major', linestyle='--', color='gray', alpha=0.7, zorder=0)
    width = 0.44

    plt.bar(xPos - width/2, LODON, width, edgecolor='black', hatch='//', label='LOD Zapnuto', zorder=2)
    plt.bar(xPos + width/2, LODOFF, width, edgecolor='black', label='LOD Vypnuto', zorder=2)

    # number on top of each bar
    for i in range(len(xLabels)):
        plt.text(xPos[i] - width/2, LODON[i] + 1, f'{LODON[i]:.1f}', ha='center', va='bottom', fontsize=9)
        plt.text(xPos[i] + width/2, LODOFF[i] + 1, f'{LODOFF[i]:.1f}', ha='center', va='bottom', fontsize=9)

    formattedLabels = [f"{int(x):,}".replace(',', ' ') for x in xLabels]

    plt.xticks(xPos, formattedLabels, rotation=30)
    plt.title('Vliv LOD na čas vykreslení snímku (GPU výpočetní režim)', pad=20)
    plt.ylabel('Čas vykreslení snímku [ms]', labelpad=10)
    plt.xlabel('Počet instancí modelu ve scéně', labelpad=10)

    plt.legend()
    plt.tight_layout()
    plt.savefig('LODINFLUENCEGPU.svg')
    plt.close()
    print("Done...")

def plotLODEnablingCPU(df):
    print("Plotting FPS to instances with LOD Enabled / Disabled ...")

    # take each unique instance count
    xLabels = sorted(df['Instances'].unique())
    xPos = np.arange(len(xLabels))

    gpuOnly = df[(df['GPULOD'] == 'CPU') & (df['GPUSpiral'] == 'CPU')]
    LODON = gpuOnly[gpuOnly['EnableLOD'] == 'Enabled']['AvgFrameTimeMs'].values
    LODOFF = gpuOnly[gpuOnly['EnableLOD'] == 'Disabled']['AvgFrameTimeMs'].values

    # plot
    plt.figure(figsize=(12, 8))
    plt.grid(axis='y', which='major', linestyle='--', color='gray', alpha=0.7, zorder=0)
    width = 0.44

    plt.bar(xPos - width/2, LODON, width, edgecolor='black', hatch='//', label='LOD Zapnuto', zorder=2)
    plt.bar(xPos + width/2, LODOFF, width, edgecolor='black', label='LOD Vypnuto', zorder=2)

    # number on top of each bar
    for i in range(len(xLabels)):
        plt.text(xPos[i] - width/2, LODON[i] + 1, f'{LODON[i]:.1f}', ha='center', va='bottom', fontsize=9)
        plt.text(xPos[i] + width/2, LODOFF[i] + 1, f'{LODOFF[i]:.1f}', ha='center', va='bottom', fontsize=9)

    formattedLabels = [f"{int(x):,}".replace(',', ' ') for x in xLabels]

    plt.xticks(xPos, formattedLabels, rotation=30)
    plt.title('Vliv LOD na čas vykreslení snímku (CPU výpočetní režim)', pad=20)
    plt.ylabel('Čas vykreslení snímku [ms]', labelpad=10)
    plt.xlabel('Počet instancí modelu ve scéně', labelpad=10)

    plt.legend()
    plt.tight_layout()
    plt.savefig('LODINFLUENCECPU.svg')
    plt.close()
    print("Done...")

def plotSceneFaces(df):
    print("Plotting Scene Faces with LOD Enabled / Disabled ...")

    # pick 5 representative instances, 
    instances = sorted(df['Instances'].unique())
    if len(instances) > 5:
        idx = np.linspace(0, len(instances) - 1, 5, dtype=int)
        selectedInstances = [instances[i] for i in idx]
    else:
        selectedInstances = instances

    df = df[df['Instances'].isin(selectedInstances)]

    # take GPU only data for LOD ON and OFF (more stable for number of drwan faces)
    gpuOnly = df[(df['GPULOD'] == 'GPU') & (df['GPUSpiral'] == 'GPU')]
    
    # divide for milions of faces
    facesON = gpuOnly[gpuOnly['EnableLOD'] == 'Enabled']['SceneFaces'].values / 1000000.0
    facesOFF = gpuOnly[gpuOnly['EnableLOD'] == 'Disabled']['SceneFaces'].values / 1000000.0

    xLabels = sorted(df['Instances'].unique())
    xPos = np.arange(len(xLabels))
    width = 0.35

    # plot
    plt.figure(figsize=(12, 8))
    plt.grid(axis='y', which='major', linestyle='--', color='gray', alpha=0.7, zorder=0)
    plt.bar(xPos - width/2, facesON, width, edgecolor='black', hatch='//', label='LOD Zapnuto', zorder=2)
    plt.bar(xPos + width/2, facesOFF, width, edgecolor='black', label='LOD Vypnuto', zorder=2)

    # number on top of each bar
    for i in range(len(xLabels)):
        plt.text(xPos[i] - width/2, facesON[i] + 5, f'{facesON[i]:.0f}M', ha='center', va='bottom', fontsize=9)
        plt.text(xPos[i] + width/2, facesOFF[i] + 5, f'{facesOFF[i]:.0f}M', ha='center', va='bottom', fontsize=9)

    formattedLabels = [f"{int(x):,}".replace(',', ' ') for x in xLabels]

    plt.xticks(xPos, formattedLabels, rotation=30)
    plt.title('Počty vykreslených trojúhelníků ve scéně', pad=20)
    plt.ylabel('Počet vykreslených trojúhelníků [miliony]', labelpad=10)
    plt.xlabel('Počet instancí modelu ve scéně', labelpad=10)

    plt.legend()
    plt.tight_layout()
    plt.savefig('DRAWNFACESLOD.svg')
    plt.close()
    print("Done...")

def plotScalability(df):
    print("Plotting frame time scalability...")

    dfLODON = df[df['EnableLOD'] == 'Enabled']
    dfLODOFF = df[df['EnableLOD'] == 'Disabled']

    cpuOnlyON = dfLODON[(dfLODON['GPULOD'] == 'CPU') & (dfLODON['GPUSpiral'] == 'CPU')]['AvgFrameTimeMs'].values
    gpuLODcpuSpiralON = dfLODON[(dfLODON['GPULOD'] == 'GPU') & (dfLODON['GPUSpiral'] == 'CPU')]['AvgFrameTimeMs'].values
    gpuOnlyON = dfLODON[(dfLODON['GPULOD'] == 'GPU') & (dfLODON['GPUSpiral'] == 'GPU')]['AvgFrameTimeMs'].values

    gpuOnlyOFF = dfLODOFF[(dfLODOFF['GPULOD'] == 'GPU') & (dfLODOFF['GPUSpiral'] == 'GPU')]['AvgFrameTimeMs'].values

    xLabels = sorted(df['Instances'].unique())
    formattedLabels = [f"{int(x):,}".replace(',', ' ') for x in xLabels]
    
    plt.figure(figsize=(12, 8))
    plt.grid(which='major', linestyle='--', color='gray', alpha=0.7)
    
    # mix LOD ON
    plt.plot(xLabels, cpuOnlyON, marker='o', linewidth=2.5, linestyle='-', label='LOD Zapnuto, CPU pozice na spirále, CPU výběr LOD ')
    plt.plot(xLabels, gpuLODcpuSpiralON, marker='v', linewidth=2.5, linestyle='-', label='LOD Zapnuto, CPU pozice na spirále, GPU výběr LOD')
    plt.plot(xLabels, gpuOnlyON, marker='s', linewidth=2.5, linestyle='-', label='LOD Zapnuto, GPU pozice na spirále, GPU výběr LOD')
    
    # GPU only LOD OFF
    plt.plot(xLabels, gpuOnlyOFF, marker='X', linewidth=2.5, linestyle='--', color='#d62728', label='LOD Vypnuto, GPU pozice na spirále, GPU výběr LOD')

    plt.title('Závislost doby vykreslení snímku na počtu instancí pro různé režimy', pad=20)
    plt.ylabel('Čas vykreslení snímku [ms]', labelpad=10)
    plt.xlabel('Počet instancí modelu ve scéně', labelpad=10)

    plt.xticks(xLabels, formattedLabels, rotation=30)
    plt.legend()
    plt.tight_layout()
    plt.savefig('INSTANCE_MS_SCALE.svg')
    plt.close()
    print("Done...")

def plotTargetFramerates(df):
    print("Plotting instances needed for certain framerates...")

    targets = {
        '60 FPS\n(16.6 ms)': 1000.0 / 60.0,
        '30 FPS\n(33.3 ms)': 1000.0 / 30.0,
        '15 FPS\n(66.6 ms)': 1000.0 / 15.0
    }

    modes = {
        'GPU režim, LOD Zapnuto': (df['GPULOD'] == 'GPU') & (df['GPUSpiral'] == 'GPU') & (df['EnableLOD'] == 'Enabled'),
        'CPU režim, LOD Zapnuto': (df['GPULOD'] == 'CPU') & (df['GPUSpiral'] == 'CPU') & (df['EnableLOD'] == 'Enabled'),
        'GPU režim, LOD Vypnuto': (df['GPULOD'] == 'GPU') & (df['GPUSpiral'] == 'GPU') & (df['EnableLOD'] == 'Disabled'),
        'CPU režim, LOD Vypnuto': (df['GPULOD'] == 'CPU') & (df['GPUSpiral'] == 'CPU') & (df['EnableLOD'] == 'Disabled')
    }

    results = {mode: [] for mode in modes}
    target_labels = list(targets.keys())

    # for each mode, find the maximum number of instances that can achieve the target ms
    for modeName, condition in modes.items():
        modeData = df[condition].sort_values(by='AvgFrameTimeMs')
        yTimes = modeData['AvgFrameTimeMs'].values
        xInstances = modeData['Instances'].values

        for targetName, target_ms in targets.items():
            if len(yTimes) == 0:
                # no data for this mode, append 0
                results[modeName].append(0)
            elif yTimes[0] > target_ms:
                # target ms is faster than the fastest time, extrapolate linearly from the first two points
                extrapolated_inst = (target_ms / yTimes[0]) * xInstances[0]
                results[modeName].append(extrapolated_inst)
            elif yTimes[-1] < target_ms:
                # target ms is slower than the slowest time, extrapolate linearly from the last two points
                slope = (xInstances[-1] - xInstances[-2]) / (yTimes[-1] - yTimes[-2])
                extrapolated_inst = xInstances[-1] + slope * (target_ms - yTimes[-1])
                results[modeName].append(extrapolated_inst)
            else:
                # interpolate between the two points that surround the target_ms
                max_inst = np.interp(target_ms, yTimes, xInstances)
                results[modeName].append(max_inst)

    # plot
    xPos = np.arange(len(target_labels))
    width = 0.2
    offsets = [-1.5, -0.5, 0.5, 1.5]
    hatches = ['//', '//', '', '']

    plt.figure(figsize=(12, 8))
    plt.grid(axis='y', which='major', linestyle='--', color='gray', alpha=0.7, zorder=0)

    # plot bars with values on top
    for i, (modeName, data) in enumerate(results.items()):
        dataMills = np.array(data) / 1000000.0
        
        plt.bar(xPos + offsets[i]*width, dataMills, width, edgecolor='black', hatch=hatches[i], label=modeName, zorder=2)

        for j in range(len(target_labels)):
            if dataMills[j] > 0:
                plt.text(xPos[j] + offsets[i]*width, dataMills[j] + 0.1, f'{dataMills[j]:.1f}M', ha='center', va='bottom', fontsize=9, rotation=0)

    plt.xticks(xPos, target_labels)
    plt.title('Maximální počet instancí k dosažení cílové snímkové frekvence', pad=20)
    plt.ylabel('Počet instancí [miliony]', labelpad=10)
    plt.xlabel('Snímková frekvence', labelpad=10)

    plt.legend()
    plt.tight_layout()
    plt.savefig('INSTANCES_TO_FPS.svg')
    plt.close()
    print("Done...")

def main():
    try:
        df = pd.read_csv("spiralBenchmark.csv")
    except FileNotFoundError:
        print("spiralBenchmark.csv file has not been found")
        return

    df = df.sort_values(by='Instances', ascending=True).reset_index(drop=True)
    plt.style.use('tableau-colorblind10')
    plt.rcParams.update({'axes.edgecolor': 'black', 'font.size': 16, 'axes.titlesize': 20, 'axes.labelsize': 18})

    plotFrameTimeInstances(df)
    plotLODEnablingCPU(df)
    plotLODEnablingGPU(df)
    plotSceneFaces(df)
    plotScalability(df)
    plotTargetFramerates(df)
    

if __name__ == '__main__':
    main()
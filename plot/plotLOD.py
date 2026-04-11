# Author: Jiri Pocarovsky
# File: plotLOD.py
# 
# This script plots graphs corresponding to the Spiral scene benchmark results.
#
# Plots:
# 1. Frame time to instances with different GPU/CPU compute configurations
# 2. FPS to instances with LOD Enabled / Disabled for GPU compute configuration
# 3. Scene faces to instances with LOD Enabled / Disabled for GPU compute configuration
# 4. Compute speedup ratio (CPU vs GPU) for LOD Enabled configuration

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
    plt.figure(figsize=(10, 7))
    plt.grid(which='major', linestyle='--', color='gray', alpha=0.7)
    
    plt.plot(xLabels, cpuOnly, marker='o', linewidth=2, linestyle='-', label='CPU pozice na spirále / CPU výběr LOD')
    plt.plot(xLabels, gpuLODcpuSpiral, marker='v', linewidth=2, linestyle='-', label='CPU pozice na spirále / GPU výběr LOD')
    plt.plot(xLabels, gpuOnly, marker='s', linewidth=2, linestyle='-', label='GPU pozice na spirále / GPU výběr LOD')

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
    plt.figure(figsize=(10, 7))
    plt.grid(axis='y', which='major', linestyle='--', color='gray', alpha=0.7, zorder=0)
    width = 0.35

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
    plt.figure(figsize=(10, 7))
    plt.grid(axis='y', which='major', linestyle='--', color='gray', alpha=0.7, zorder=0)
    width = 0.35

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

    # graph is clear even with 1 milion instances, so filter out higher instance counts for better visibility
    df = df[df['Instances'].isin([200000, 400000, 600000, 800000, 1000000])]

    # take GPU only data for LOD ON and OFF (more stable for number of drwan faces)
    gpuOnly = df[(df['GPULOD'] == 'GPU') & (df['GPUSpiral'] == 'GPU')]
    
    # divide for milions of faces
    facesON = gpuOnly[gpuOnly['EnableLOD'] == 'Enabled']['SceneFaces'].values / 1000000.0
    facesOFF = gpuOnly[gpuOnly['EnableLOD'] == 'Disabled']['SceneFaces'].values / 1000000.0

    xLabels = sorted(df['Instances'].unique())
    xPos = np.arange(len(xLabels))
    width = 0.35

    # plot
    plt.figure(figsize=(10, 7))
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

def plotSpeedupRatio(df):
    print("Plotting Compute Speedup Ratio ...")
    
    dfLODON = df[df['EnableLOD'] == 'Enabled']
    cpuON = dfLODON[(dfLODON['GPULOD'] == 'CPU') & (dfLODON['GPUSpiral'] == 'CPU')]['AvgFrameTimeMs'].values
    gpuON = dfLODON[(dfLODON['GPULOD'] == 'GPU') & (dfLODON['GPUSpiral'] == 'GPU')]['AvgFrameTimeMs'].values

    # get relative speedup of GPU only vs CPU only, LOD ON
    speedupLODON = cpuON / gpuON

    dfLODOFF = df[df['EnableLOD'] == 'Disabled']
    cpuOFF = dfLODOFF[(dfLODOFF['GPULOD'] == 'CPU') & (dfLODOFF['GPUSpiral'] == 'CPU')]['AvgFrameTimeMs'].values
    gpuOFF = dfLODOFF[(dfLODOFF['GPULOD'] == 'GPU') & (dfLODOFF['GPUSpiral'] == 'GPU')]['AvgFrameTimeMs'].values

    # get relative speedup of GPU only vs CPU only, LOD OFF
    speedupLODOFF = cpuOFF / gpuOFF

    xLabels = sorted(df['Instances'].unique())
    formattedLabels = [f"{int(x):,}".replace(',', ' ') for x in xLabels]

    # set y-ticks to 0.25 increments up to the maximum speedup
    maxSpeed = max(np.max(speedupLODON), np.max(speedupLODOFF))
    maxTick = np.ceil(maxSpeed * 4) / 4
    yTicks = np.arange(1.0, maxTick + 0.25, 0.25)
    yLabels = [f"{y:.2f}x" for y in yTicks]

    # plot
    plt.figure(figsize=(10, 7))
    plt.grid(which='major', linestyle='--', color='gray', alpha=0.7, zorder = 0)

    plt.plot(xLabels, speedupLODON, marker='o', linewidth=2.5, label='Výpočty na GPU (LOD Zapnuto)', zorder=3)
    plt.plot(xLabels, speedupLODOFF, marker='s', linewidth=2.5, label='Výpočty na GPU (LOD Vypnuto)', zorder=3)
    plt.axhline(y=1, linestyle='--', color = 'red', linewidth=1.5, label='Výkon CPU')

    plt.yticks(yTicks, yLabels)
    plt.xticks(xLabels, formattedLabels, rotation=30)

    plt.title('Zrychlení při použití výpočtu na GPU', pad=20)
    plt.ylabel('Relativní zrychlení oproti CPU', labelpad=10)
    plt.xlabel('Počet instancí modelu ve scéně', labelpad=10)

    plt.legend()
    plt.tight_layout()
    plt.savefig('SPEEDUPCPUGPU.svg')
    plt.close()
    print("Done...")

def main():
    try:
        df = pd.read_csv("spiralBenchmark.csv")
    except FileNotFoundError:
        print("spiralBenchmark.csv file has not been found")
        return

    df = df.sort_values(by='Instances', ascending=True).reset_index(drop=True)
    df['AvgFPS'] = np.where(df['AvgFrameTimeMs'] > 0, 1000.0 / df['AvgFrameTimeMs'], 0)

    plt.style.use('tableau-colorblind10')
    plt.rcParams.update({'axes.edgecolor': 'black', 'font.size': 12, 'axes.titlesize': 14, 'axes.labelsize': 12})

    plotFrameTimeInstances(df)
    plotLODEnablingCPU(df)
    plotLODEnablingGPU(df)
    plotSceneFaces(df)
    plotSpeedupRatio(df)
    

if __name__ == '__main__':
    main()
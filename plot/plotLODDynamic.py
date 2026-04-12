import os
import matplotlib.pyplot as plt
import pandas as pd
import numpy as np

def plotLODTransitions(df):
    print("Plotting LOD Transitions...")
    plt.figure(figsize=(10, 7))
    
    maxFaces = (df['Instances'].iloc[0] * 960) / 1000000.0
    
    # LOD 0 = 960 faces, LOD 1 = 480 faces, LOD 2 = 240 faces, LOD 3 = 120 faces
    x = df['CAMZ'].values
    lod0 = df['LOD0'].values
    lod1 = df['LOD1'].values
    lod2 = df['LOD2'].values
    lod3 = df['LOD3'].values
    
    plt.grid(which='major', linestyle='--', color='gray', alpha=0.7, zorder=0)
    
    # colors correpsonding to LOD levels in the wireframe
    colors = ['#2ca02c', '#ff7f0e', '#1f77b4', '#d62728']
    plt.stackplot(x, lod0, lod1, lod2, lod3, labels=['LOD 0', 'LOD 1', 'LOD 2', 'LOD 3'], colors=colors, alpha=0.8, zorder=3)
    
    # vertical lines for LOD thresholds with text
    maxInstances = max(lod0 + lod1 + lod2 + lod3)
    texty = maxInstances * 0.95
    plt.axvline(x=1000, color='black', linestyle=':', linewidth=2, alpha=0.6, zorder=3)
    plt.text(950, texty, 'Práh LOD 0', rotation=0, va='top', ha='right', alpha=0.8, fontsize=11, zorder=3)
    plt.axvline(x=2000, color='black', linestyle=':', linewidth=2, alpha=0.6, zorder=3)
    plt.text(1950, texty, 'Práh LOD 1', rotation=0, va='top', ha='right', alpha=0.8, fontsize=11, zorder=3)
    plt.axvline(x=4000, color='black', linestyle=':', linewidth=2, alpha=0.6, zorder=3)
    plt.text(3950, texty, 'Práh LOD 2', rotation=0, va='top', ha='right', alpha=0.8, fontsize=11, zorder=3)

    plt.title('Přechody mezi jednotlivými úrovněmi detailu', pad=20)
    plt.xlabel('Vzdálenost kamery od středu spirály [d.j.]', labelpad=10)
    plt.ylabel('Počet instancí', labelpad=10)
    
    plt.xlim(min(x), max(x))
    plt.ylim(0, max(lod0 + lod1 + lod2 + lod3))
    
    plt.legend(loc='upper right')
    plt.tight_layout()
    plt.savefig('LODTRANS.svg')
    plt.close()
    print("Done...")

def plotSceneFacesDynamic(df):
    print("Plotting Scene Faces Dynamic...")
    plt.figure(figsize=(10, 7))
    
    # get values for x and y axes
    x = df['CAMZ'].values
    faces = df['SceneFaces'].values / 1000000.0 
    maxFaces = (df['Instances'].iloc[0] * 960) / 1000000.0
    print(f"LOD OFF: {maxFaces}")
    
    plt.grid(which='major', linestyle='--', color='gray', alpha=0.7, zorder=0)
    
    plt.plot(x, faces, marker='', linewidth=3, label='LOD zapnut', zorder=3)
    plt.fill_between(x, faces, alpha=0.15, zorder=2)
    
    plt.axhline(y=maxFaces, color='red', linestyle='--', linewidth=2, label='LOD vypnut')

    plt.axvline(x=1000, color='black', linestyle=':', linewidth=2, alpha=0.6, zorder=1)
    plt.text(1050, maxFaces * 0.05, 'Práh LOD 0', rotation=0, va='top', ha='left', alpha=0.8, fontsize=11)
    plt.axvline(x=2000, color='black', linestyle=':', linewidth=2, alpha=0.6, zorder=1)
    plt.text(2050, maxFaces * 0.05, 'Práh LOD 1', rotation=0, va='top', ha='left', alpha=0.8, fontsize=11)
    plt.axvline(x=4000, color='black', linestyle=':', linewidth=2, alpha=0.6, zorder=1)
    plt.text(4050, maxFaces * 0.05, 'Práh LOD 2', rotation=0, va='top', ha='left', alpha=0.8, fontsize=11)

    plt.title('Vliv LOD na geometrickou složitost scény při pohybu kamery', pad=20)
    plt.xlabel('Vzdálenost kamery od středu spirály [d.j.]', labelpad=10)
    plt.ylabel('Počet vykreslených trojúhelníků [miliony]', labelpad=10)
    
    plt.xlim(min(x), max(x))
    plt.ylim(0, maxFaces + 0.5)
    
    plt.legend()
    plt.tight_layout()
    plt.savefig('MOVINGCAM.svg')
    plt.close()
    print("Done...")

def main():
    try:
        df = pd.read_csv("spiralBenchmarkMoving.csv")
    except FileNotFoundError:
        print("spiralBenchmarkMoving.csv file has not been found")
        return

    plt.style.use('tableau-colorblind10')
    plt.rcParams.update({'axes.edgecolor': 'black', 'font.size': 12, 'axes.titlesize': 14, 'axes.labelsize': 12})

    plotLODTransitions(df)
    plotSceneFacesDynamic(df)

if __name__ == '__main__':
    main()
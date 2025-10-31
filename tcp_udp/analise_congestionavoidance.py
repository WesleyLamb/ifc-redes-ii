#!/usr/bin/env python3
import matplotlib.pyplot as plt
import pandas as pd
import numpy as np

def plot_congestion_analysis():
    try:
        # Leitura dos dados
        data = pd.read_csv('cwnd_monitor.txt', sep='\t', comment='#', 
                          names=['Tempo', 'TCP1_cwnd', 'TCP1_ssthresh', 'TCP2_cwnd', 'TCP2_ssthresh'])
        
        # Configuração da figura
        plt.figure(figsize=(15, 10))
        
        # Subplot 1: Evolução das janelas
        plt.subplot(2, 1, 1)
        plt.plot(data['Tempo'], data['TCP1_cwnd'], 'r-', linewidth=2, label='Fluxo Vermelho (cwnd)')
        plt.plot(data['Tempo'], data['TCP1_ssthresh'], 'r--', alpha=0.7, label='Fluxo Vermelho (ssthresh)')
        plt.plot(data['Tempo'], data['TCP2_cwnd'], 'b-', linewidth=2, label='Fluxo Azul (cwnd)')
        plt.plot(data['Tempo'], data['TCP2_ssthresh'], 'b--', alpha=0.7, label='Fluxo Azul (ssthresh)')
        
        # Marcos importantes
        plt.axvline(x=4, color='orange', linestyle=':', alpha=0.8, label='Transição para CA')
        plt.axvline(x=8, color='red', linestyle=':', alpha=0.8, label='Início Congestionamento')
        plt.axvline(x=18, color='green', linestyle=':', alpha=0.8, label='Fim Congestionamento')
        
        plt.xlabel('Tempo (s)')
        plt.ylabel('Tamanho da Janela (segmentos)')
        plt.title('Evolução das Janelas TCP - Slow Start vs Congestion Avoidance')
        plt.legend(bbox_to_anchor=(1.05, 1), loc='upper left')
        plt.grid(True, alpha=0.3)
        
        # Subplot 2: Throughput estimado
        plt.subplot(2, 1, 2)
        
        # Cálculo aproximado do throughput (cwnd * MSS / RTT)
        RTT = 0.05  # 50ms aproximado
        MSS = 1000  # bytes
        
        throughput1 = (data['TCP1_cwnd'] * MSS * 8) / (RTT * 1000000)  # Mbps
        throughput2 = (data['TCP2_cwnd'] * MSS * 8) / (RTT * 1000000)  # Mbps
        
        plt.plot(data['Tempo'], throughput1, 'r-', linewidth=2, label='Throughput Fluxo Vermelho')
        plt.plot(data['Tempo'], throughput2, 'b-', linewidth=2, label='Throughput Fluxo Azul')
        plt.plot(data['Tempo'], throughput1 + throughput2, 'k--', alpha=0.7, label='Throughput Total')
        
        plt.axhline(y=2, color='gray', linestyle='-', alpha=0.5, label='Capacidade Link (2 Mbps)')
        
        plt.xlabel('Tempo (s)')
        plt.ylabel('Throughput (Mbps)')
        plt.title('Throughput Estimado dos Fluxos TCP')
        plt.legend()
        plt.grid(True, alpha=0.3)
        
        plt.tight_layout()
        plt.savefig('congestion_analysis.png', dpi=300, bbox_inches='tight')
        plt.show()
        
        # Estatísticas
        print("\n📊 ANÁLISE ESTATÍSTICA")
        print("=" * 50)
        print(f"Janela máxima TCP1: {data['TCP1_cwnd'].max():.1f} segmentos")
        print(f"Janela máxima TCP2: {data['TCP2_cwnd'].max():.1f} segmentos")
        print(f"Throughput médio TCP1: {throughput1.mean():.2f} Mbps")
        print(f"Throughput médio TCP2: {throughput2.mean():.2f} Mbps")
        print(f"Utilização média do link: {((throughput1 + throughput2).mean() / 2 * 100):.1f}%")
        
    except FileNotFoundError:
        print("❌ Arquivo cwnd_monitor.txt não encontrado!")
        print("Execute primeiro a simulação NS2.")
    except Exception as e:
        print(f"❌ Erro na análise: {e}")

if __name__ == "__main__":
    plot_congestion_analysis()
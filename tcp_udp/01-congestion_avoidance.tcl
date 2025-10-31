# ============================================================================
# Simulação TCP Congestion Avoidance com Visualização NAM
# Autor: Prof. Angelita - Instituto Federal Catarinense
# Objetivo: Demonstrar Slow Start vs Congestion Avoidance visualmente
# ============================================================================

# Criação do simulador
set ns [new Simulator]

# ============================================================================
# CONFIGURAÇÃO DE CORES E VISUALIZAÇÃO
# ============================================================================

# Cores para os fluxos
$ns color 1 Red      
$ns color 2 Blue     
$ns color 3 Green    
$ns color 4 Yellow   

# Arquivos de saída
set tracefile [open "congestion_avoidance.tr" w]
$ns trace-all $tracefile

set namfile [open "congestion_avoidance.nam" w]
$ns namtrace-all $namfile

# ============================================================================
# CRIAÇÃO DA TOPOLOGIA
# ============================================================================

# Criação dos nós
set n0 [$ns node]    
set n1 [$ns node]    
set r0 [$ns node]    
set r1 [$ns node]    
set n2 [$ns node]    
set n3 [$ns node]    

# Configuração visual dos nós
$n0 color "red"
$n1 color "blue"
$r0 color "black"
$r1 color "black"
$n2 color "red"
$n3 color "blue"

# Labels dos nós para identificação
$n0 label "Cliente-A"
$n1 label "Cliente-B"
$r0 label "Router-1"
$r1 label "Router-2"
$n2 label "Servidor-A"
$n3 label "Servidor-B"

# ============================================================================
# CONFIGURAÇÃO DOS LINKS
# ============================================================================

# Links dos hosts aos roteadores (alta capacidade)
$ns duplex-link $n0 $r0 10Mb 5ms DropTail
$ns duplex-link $n1 $r0 10Mb 5ms DropTail
$ns duplex-link $r1 $n2 10Mb 5ms DropTail
$ns duplex-link $r1 $n3 10Mb 5ms DropTail

# Link gargalo entre roteadores (PONTO CRÍTICO)
$ns duplex-link $r0 $r1 2Mb 20ms DropTail
$ns queue-limit $r0 $r1 15

# ============================================================================
# POSICIONAMENTO PARA VISUALIZAÇÃO NAM
# ============================================================================

# Posicionamento estratégico dos nós
$ns duplex-link-op $n0 $r0 orient right-down
$ns duplex-link-op $n1 $r0 orient right-up
$ns duplex-link-op $r0 $r1 orient right
$ns duplex-link-op $r1 $n2 orient right-up
$ns duplex-link-op $r1 $n3 orient right-down

# Monitoramento visual do link gargalo
$ns duplex-link-op $r0 $r1 queuePos 0.5
$ns duplex-link-op $r0 $r1 color "red"

# ============================================================================
# CONFIGURAÇÃO TCP - FLUXO VERMELHO (n0 -> n2)
# ============================================================================

# Agente TCP Reno no cliente n0
set tcp1 [new Agent/TCP/Reno]
$tcp1 set class_ 1
$tcp1 set window_ 32
$tcp1 set packetSize_ 1000
$tcp1 set ssthresh_ 8
$ns attach-agent $n0 $tcp1

# Sink no servidor n2
set sink1 [new Agent/TCPSink]
$ns attach-agent $n2 $sink1
$ns connect $tcp1 $sink1

# Aplicação FTP para fluxo contínuo
set ftp1 [new Application/FTP]
$ftp1 attach-agent $tcp1

# ============================================================================
# CONFIGURAÇÃO TCP - FLUXO AZUL (n1 -> n3)
# ============================================================================

# Agente TCP Reno no cliente n1
set tcp2 [new Agent/TCP/Reno]
$tcp2 set class_ 2
$tcp2 set window_ 32
$tcp2 set packetSize_ 1000
$tcp2 set ssthresh_ 8
$ns attach-agent $n1 $tcp2

# Sink no servidor n3
set sink2 [new Agent/TCPSink]
$ns attach-agent $n3 $sink2
$ns connect $tcp2 $sink2

# Aplicação FTP
set ftp2 [new Application/FTP]
$ftp2 attach-agent $tcp2

# ============================================================================
# MONITORAMENTO E ANOTAÇÕES VISUAIS
# ============================================================================

# Arquivo para dados de monitoramento
set cwndfile [open "cwnd_monitor.txt" w]
puts $cwndfile "# Tempo\tTCP1_cwnd\tTCP1_ssthresh\tTCP2_cwnd\tTCP2_ssthresh"

# Procedimento de monitoramento
proc monitor_cwnd {} {
    global ns tcp1 tcp2 cwndfile
    
    set time [$ns now]
    set cwnd1 [$tcp1 set cwnd_]
    set ssthresh1 [$tcp1 set ssthresh_]
    set cwnd2 [$tcp2 set cwnd_]
    set ssthresh2 [$tcp2 set ssthresh_]
    
    puts $cwndfile "$time\t$cwnd1\t$ssthresh1\t$cwnd2\t$ssthresh2"
    
    # Reagenda monitoramento
    if {$time < 30.0} {
        $ns at [expr $time + 0.1] "monitor_cwnd"
    }
}

# Procedimento para adicionar anotações visuais no NAM
proc add_annotation {time text} {
    global ns
    $ns at $time [list $ns trace-annotate $text]
}

# ============================================================================
# GERAÇÃO DE CONGESTIONAMENTO CONTROLADO
# ============================================================================

# Tráfego CBR para simular congestionamento adicional
set udp_congest [new Agent/UDP]
$ns attach-agent $n0 $udp_congest

set null_congest [new Agent/Null]
$ns attach-agent $n2 $null_congest

$ns connect $udp_congest $null_congest

set cbr_congest [new Application/Traffic/CBR]
$cbr_congest attach-agent $udp_congest
$cbr_congest set packetSize_ 500
$cbr_congest set rate_ 1Mb

# ============================================================================
# CRONOGRAMA DE EVENTOS COM ANOTAÇÕES
# ============================================================================

# Anotações iniciais
$ns at 0.0 [list add_annotation 0.0 "=== INICIO DA SIMULACAO ==="]
$ns at 0.5 [list add_annotation 0.5 "Iniciando conexoes TCP"]

# Início das conexões TCP
$ns at 1.0 [list $ftp1 start]
$ns at 1.0 [list add_annotation 1.0 "FLUXO VERMELHO: Slow Start iniciado"]

$ns at 2.0 [list $ftp2 start]
$ns at 2.0 [list add_annotation 2.0 "FLUXO AZUL: Slow Start iniciado"]

# Início do monitoramento
$ns at 1.0 "monitor_cwnd"

# Marcos do Slow Start
$ns at 4.0 [list add_annotation 4.0 "Transicao para Congestion Avoidance"]

# Introdução de congestionamento adicional
$ns at 8.0 [list $cbr_congest start]
$ns at 8.0 [list add_annotation 8.0 "CONGESTIONAMENTO: Trafego adicional iniciado"]

# Marcos do Congestion Avoidance
$ns at 10.0 [list add_annotation 10.0 "Congestion Avoidance: Crescimento linear"]

# Simulação de perda/congestionamento severo
$ns at 15.0 [list add_annotation 15.0 "Deteccao de perdas: Reducao da janela"]

# Parada do tráfego adicional
$ns at 18.0 [list $cbr_congest stop]
$ns at 18.0 [list add_annotation 18.0 "Congestionamento reduzido"]

# Observação da recuperação
$ns at 20.0 [list add_annotation 20.0 "Recuperacao: Novo ciclo Slow Start"]

# Finalização
$ns at 25.0 [list $ftp1 stop]
$ns at 25.0 [list $ftp2 stop]
$ns at 25.0 [list add_annotation 25.0 "Conexoes finalizadas"]

$ns at 30.0 "finish"

# ============================================================================
# PROCEDIMENTO DE FINALIZAÇÃO
# ============================================================================

proc finish {} {
    global ns tracefile namfile cwndfile
    
    $ns flush-trace
    close $tracefile
    close $namfile
    close $cwndfile
    
    puts "\n============================================"
    puts "SIMULACAO CONCLUIDA COM SUCESSO!"
    puts "============================================"
    puts "Arquivos gerados:"
    puts "• congestion_avoidance.nam - Animacao NAM"
    puts "• congestion_avoidance.tr  - Trace completo"
    puts "• cwnd_monitor.txt    - Dados das janelas"
    puts "============================================"
    exec nam congestion_avoidance.nam &
    exit 0
}

# ============================================================================
# INFORMAÇÕES INICIAIS
# ============================================================================

puts "\n🚀 SIMULACAO TCP CONGESTION AVOIDANCE"
puts "======================================"
puts "Topologia:"
puts "• n0 (Cliente-A) <-> r0 <-> r1 <-> n2 (Servidor-A) \[FLUXO VERMELHO\]"
puts "• n1 (Cliente-B) <-> r0 <-> r1 <-> n3 (Servidor-B) \[FLUXO AZUL\]"
puts "\nObservacoes no NAM:"
puts "• Slow Start: Crescimento exponencial inicial"
puts "• Congestion Avoidance: Crescimento linear apos ssthresh"
puts "• Competicao entre fluxos no link gargalo"
puts "• Padrao dentes de serra nas perdas"
puts "\nDuracao: 30 segundos simulados"
puts "======================================"

# Execução da simulação
$ns run

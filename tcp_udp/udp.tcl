# Cria uma instância do simulador
set ns [new Simulator]

# Configuração de cores para os pacotes UDP
$ns color 1 Blue
$ns color 2 Red

# Configuração do arquivo de rastreamento (opcional)
set tracefile [open arquivo1.tr w]
$ns trace-all $tracefile

# Configuração do arquivo de animação (opcional)
set namfile [open arquivo1.nam w]
$ns namtrace-all $namfile

# Criação de nós
set n0 [$ns node]
set n1 [$ns node]
set n2 [$ns node]
set n3 [$ns node]
set n4 [$ns node]
set n5 [$ns node]

# Marcar os nós com tags
$n0 label "emissor1"
$n4 label "receptor1"
$n1 label "emissor2"
$n5 label "receptor2"

# Criação de links entre os nós
$ns duplex-link $n0 $n2 2Mb 30ms DropTail
$ns duplex-link $n3 $n2 2Mb 30ms DropTail
$ns duplex-link $n1 $n2 2Mb 30ms DropTail
$ns duplex-link $n3 $n4 2Mb 30ms DropTail
$ns duplex-link $n3 $n5 2Mb 30ms DropTail

# Organização dos nós na simulação
$ns duplex-link-op $n0 $n2 orient right-down
$ns duplex-link-op $n3 $n2 orient left
$ns duplex-link-op $n1 $n2 orient right-up
$ns duplex-link-op $n3 $n4 orient right-up
$ns duplex-link-op $n3 $n5 orient right-down

# Configuração da camada de transporte (UDP) - comunicação 1 (n1 -> n4)
set udp0 [new Agent/UDP]
$udp0 set fid_ 1     ;# Define o Flow ID
$ns attach-agent $n0 $udp0

set null0 [new Agent/Null]
$ns attach-agent $n4 $null0

$ns connect $udp0 $null0

# Criação de uma aplicação CBR (Constant Bit Rate) que gera tráfego UDP
set cbr [new Application/Traffic/CBR]
$cbr set packetSize_ 500    ;# Tamanho do pacote em bytes
$cbr set interval_ 0.005    ;# Intervalo entre pacotes
$cbr attach-agent $udp0

# Configuração da camada de transporte (UDP) - comunicação 2 (n2 -> n0)
set udp1 [new Agent/UDP]
$udp1 set fid_ 2     ;# Define o Flow ID
$ns attach-agent $n1 $udp1

set null1 [new Agent/Null]
$ns attach-agent $n5 $null1

$ns connect $udp1 $null1

# Criação de uma aplicação CBR (Constant Bit Rate) que gera tráfego UDP
set cbr1 [new Application/Traffic/CBR]
$cbr1 set packetSize_ 500    ;# Tamanho do pacote em bytes
$cbr1 set interval_ 0.005    ;# Intervalo entre pacotes
$cbr1 attach-agent $udp1


# Agendamento do início e término da aplicação
$ns at 0.3 "$cbr start"
$ns at 0.5 "$cbr1 start"
$ns at 4.5 "$cbr stop"
$ns at 4.0 "$cbr1 stop"

# Agendamento da finalização da simulação
$ns at 5.0 "finish"

# Função para finalização
proc finish {} {
    global ns tracefile namfile
    $ns flush-trace
    close $tracefile
    close $namfile
    exec nam arquivo1.nam &
    exit 0
}

# Execução da simulação
$ns run

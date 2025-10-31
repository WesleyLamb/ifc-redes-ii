# Simulação de comunicação simples e confiável (canal confiável)
# Baseado em UDP e CBR, com canal sem perda e sem atrasos adicionais (apenas propagação)
# Autor: Angelita Rettore de Araujo
# Última alteração: 2025-19-08
# versão 1.0

set ns [new Simulator]

set n0 [$ns node]
set n1 [$ns node]

$ns at 0.0 "$n0 label Emissor"
$ns at 0.0 "$n1 label Receptor"

$ns color 1 blue

set nf [open 01-rdt1-canal-confiavel.nam w]
$ns namtrace-all $nf
set f [open 01-rdt1-canal-confiavel.tr w]
$ns trace-all $f

# Link Duplex: Largura de banda de 0.2Mb/s, atraso de propagação de 200ms
# O atraso de propagação é inerente ao enlace e não é um "atraso" que podemos remover.
# Para garantir "sem perda" e "sem atrasos (de fila)", aumentamos o limite da fila.
$ns duplex-link $n0 $n1 0.2Mb 200ms DropTail
$ns duplex-link-op $n0 $n1 orient right

# Aumentando o limite da fila para garantir que não haja perdas por buffer cheio
$ns queue-limit $n0 $n1 1000

# Agentes de Transporte: UDP no emissor, Null no receptor (não requer ACKs)
set udp [new Agent/UDP]
$udp set fid_ 1 ; # Identificador de fluxo para NAM
$ns attach-agent $n0 $udp

set null_sink [new Agent/Null]
$ns attach-agent $n1 $null_sink

$ns connect $udp $null_sink

# Aplicação: CBR (Constant Bit Rate) - envia dados continuamente
set cbr [new Application/Traffic/CBR]
$cbr attach-agent $udp
$cbr set packetSize_ 1000 ; # Tamanho do pacote em bytes
$cbr set rate_ 0.1Mb ; # Taxa de envio de dados (100 kbps), bem abaixo da capacidade do link (200 kbps)
                     ; # Isso ajuda a evitar congestionamento e perdas na fila.
$cbr set random_ false ; # Envio sem aleatoriedade para um comportamento mais constante

$ns at 0.1 "$cbr start"
$ns at 3.0 "$cbr stop" ; # Interrompe o envio de CBR
$ns at 3.1 "$ns detach-agent $n0 $udp ; $ns detach-agent $n1 $null_sink"
$ns at 3.5 "finish"

# Anotações para a visualização NAM
$ns at 0.0 "$ns trace-annotate \"Simulacao: Canal Confiável - Sem ACKs\""
$ns at 0.05 "$ns trace-annotate \"CBR (UDP) iniciando em 0.1s\""
$ns at 0.15 "$ns trace-annotate \"Emissor: Enviando dados continuamente\""
$ns at 0.30 "$ns trace-annotate \"Receptor: Recebendo dados continuamente\""
$ns at 3.1 "$ns trace-annotate \"CBR (UDP) terminou\""


proc finish {} {
    global ns nf f
    $ns flush-trace
    close $nf
    close $f

    puts "filtering..."
    puts "running nam..."
    exec nam 01-rdt1-canal-confiavel.nam &
    exit 0
}

$ns run
# Simulação de comunicação em canal com erros de bit
# Emissor envia, Receptor recebe - sem ACKs ou retransmissões (UDP/CBR)
# Erros de bit são introduzidos e se manifestam como perda de pacotes.
# Autor: Angelita Rettore de Araujo
# Última alteração: 2025-19-08
# versão 1.0

set ns [new Simulator]

set n0 [$ns node]
set n1 [$ns node]

$ns at 0.0 "$n1 label Emissor"
$ns at 0.0 "$n0 label Receptor"

$ns color 1 blue

set nf [open 02-rdt1-canal-com-erros.nam w]
$ns namtrace-all $nf
set f [open 02-rdt1-canal-com-erros.tr w]
$ns trace-all $f

# Link Duplex: Largura de banda de 0.5Mb/s, atraso de propagação de 100ms
$ns duplex-link $n1 $n0 0.5Mb 100ms DropTail
$ns duplex-link-op $n1 $n0 orient right

# Aumentando o limite da fila para garantir que não haja perdas por buffer cheio INICIALMENTE
# (as perdas virão agora do modelo de erro)
$ns queue-limit $n1 $n0 10000

# *** NOVIDADE: Modelo de Erro para o Canal ***
set em [new ErrorModel]
# Taxa de erro de bit (BER): 1 erro a cada 100 bits
# Ajuste este valor para controlar a frequência das perdas.
$em set rate_ 1e-2
$em set unit_ bit

# Anexa o modelo de erro a ambas as direções do link
$ns lossmodel $em $n0 $n1
$ns lossmodel $em $n1 $n0

# **********************************************

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
$cbr set rate_ 1Mb ; # Taxa de envio de dados (1 Mbps), abaixo da capacidade do link
$cbr set random_ false ; # Envio sem aleatoriedade para um comportamento mais constante

$ns at 0.1 "$cbr start"
$ns at 3.0 "$cbr stop" ; # Interrompe o envio de CBR
$ns at 3.1 "$ns detach-agent $n0 $udp ; $ns detach-agent $n1 $null_sink"
$ns at 3.5 "finish"

# Anotações para a visualização NAM
$ns at 0.0 "$ns trace-annotate \"Simulacao: Canal COM Erros de Bit (UDP)\""
$ns at 0.05 "$ns trace-annotate \"BER = 1e-5. Pacotes perdidos serao detectados!\""
$ns at 0.15 "$ns trace-annotate \"Emissor: Enviando dados continuamente\""
$ns at 0.30 "$ns trace-annotate \"Receptor: Recebendo dados (com perdas)\""
$ns at 3.1 "$ns trace-annotate \"Simulacao encerrada\""


proc finish {} {
    global ns nf f
    $ns flush-trace
    close $nf
    close $f

    puts "filtering..."
    puts "running nam..."
    exec nam 02-rdt1-canal-com-erros.nam &
    exit 0
}

$ns run
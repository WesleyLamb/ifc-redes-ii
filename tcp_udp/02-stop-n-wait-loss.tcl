# Simulação do Protocolo Stop-and-Wait com Perda de Pacotes
# Este script demonstra o comportamento do Stop-and-Wait quando um pacote (ou ACK) é perdido.
# Utiliza manipulação da fila para simular a perda e mostra a retransmissão por timeout.
# Autor: Angelita Rettore de Araujo
# Última alteração: 2025-08-19
# versão 1.0

set ns [new Simulator]

set n0 [$ns node]
set n1 [$ns node]

$ns at 0.0 "$n0 label Emissor"
$ns at 0.0 "$n1 label Receptor"

$ns color 1 blue ; 

set nf [open 02-stop-n-wait-loss.nam w]
$ns namtrace-all $nf
set f [open 02-stop-n-wait-loss.tr w]
$ns trace-all $f

$ns duplex-link $n0 $n1 0.2Mb 200ms DropTail
$ns duplex-link-op $n0 $n1 orient right
$ns duplex-link-op $n0 $n1 queuePos 0.5
$ns queue-limit $n0 $n1 10 ; # Limite da fila padrão

Agent/TCP set nam_tracevar_ true

set tcp [new Agent/TCP]
$tcp set window_ 1
$tcp set maxcwnd_ 1
$tcp set fid_ 1 ;

$ns attach-agent $n0 $tcp

set sink [new Agent/TCPSink]
$sink set fid_ 1 ; 
$ns attach-agent $n1 $sink

$ns connect $tcp $sink

set ftp [new Application/FTP]
$ftp attach-agent $tcp

$ns add-agent-trace $tcp tcp
$ns monitor-agent-trace $tcp
$tcp tracevar cwnd_


$ns at 0.1 "$ftp start"
# Simula a perda de um ACK zerando a capacidade da fila de n1 para n0 temporariamente.
# O ACK do Pacote_3 será perdido entre 1.1s e 1.5s.
$ns at 1.1 "$ns queue-limit $n1 $n0 0"
$ns at 1.5 "$ns queue-limit $n1 $n0 10" ; # Restaura o limite da fila
$ns at 3.0 "$ns detach-agent $n0 $tcp ; $ns detach-agent $n1 $sink" 
$ns at 3.5 "finish"

# Anotações para a visualização NAM
$ns at 0.0 "$ns trace-annotate \"Stop and Wait com perda de pacotes\""
$ns at 0.05 "$ns trace-annotate \"FTP inicia em 0.1\""
$ns at 0.11 "$ns trace-annotate \"Enviando SYN\""
$ns at 0.3 "$ns trace-annotate \"Enviando ACK\""
$ns at 0.51 "$ns trace-annotate \"Enviando pacote_1\""
$ns at 0.79 "$ns trace-annotate \"Enviando ACK_1\""
$ns at 0.99 "$ns trace-annotate \"Enviando pacote_2\""
$ns at 1.23 "$ns trace-annotate \"Recebendo ACK_2\""
$ns at 1.40 "$ns trace-annotate \"Enviando Pacote_3\""
$ns at 1.43 "$ns trace-annotate \"ACK do Pacote_3 perdido\""; 
$ns at 1.5 "$ns trace-annotate \"Aguardando ACK_3\""
$ns at 2.43 "$ns trace-annotate \"Enviando Pacote_3 novamente (devido ao timeout)\""
$ns at 2.67 "$ns trace-annotate \"Recebendo ACK_3\""
$ns at 2.88 "$ns trace-annotate \"Enviando Pacote_4\""
$ns at 3.1 "$ns trace-annotate \"FTP terminou\""


proc finish {} {
    global ns nf f
    $ns flush-trace
    close $nf
    close $f

    puts "filtering..."
    puts "running nam..."
    exec nam 02-stop-n-wait-loss.nam &
    exit 0
}

$ns run
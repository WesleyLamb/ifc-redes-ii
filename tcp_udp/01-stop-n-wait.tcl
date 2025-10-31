# Simulação do Protocolo Stop-and-Wait em Condições Normais
# Este script demonstra a operação básica do Stop-and-Wait sem perdas ou erros.
# Ele ilustra a troca de um pacote de dados seguido de um ACK antes do envio do próximo pacote.
# Autor: Angelita Rettore de Araujo
# Última alteração: 2025-08-19
# versão 1.0

set ns [new Simulator] ; # Cria uma nova instância do simulador

# Define os nós da rede
set n0 [$ns node] ; # Nó Emissor
set n1 [$ns node] ; # Nó Receptor

# Rótulos para identificação dos nós na visualização NAM
$ns at 0.0 "$n0 label Emissor"
$ns at 0.0 "$n1 label Receptor"

$ns color 1 blue ; # Define a cor azul para fluxos com FID (Flow ID) igual a 1

# Configuração dos arquivos de rastreamento (trace files) para NAM e análise de texto
set nf [open 01-stop-n-wait.nam w] ; # Abre arquivo .nam para visualização
$ns namtrace-all $nf ; # Habilita o rastreamento de todos os eventos para o NAM
set f [open 01-stop-n-wait.tr w] ; # Abre arquivo .tr para rastreamento detalhado
$ns trace-all $f ; # Habilita o rastreamento de todos os eventos para o .tr

# Configuração do link duplex entre os nós n0 e n1
# Largura de banda: 0.2Mb/s, Atraso de propagação: 200ms, Tipo de fila: DropTail
$ns duplex-link $n0 $n1 0.2Mb 200ms DropTail
$ns duplex-link-op $n0 $n1 orient right ; # Orienta o link visualmente para a direita no NAM
$ns queue-limit $n0 $n1 10 ; # Define o limite da fila para 10 pacotes

Agent/TCP set nam_tracevar_ true ; # Habilita variáveis de rastreamento do TCP para o NAM

# Criação e configuração do Agente TCP (Emissor)
set tcp [new Agent/TCP]
$tcp set window_ 1 ; # Define a janela de envio para 1 (característica do Stop-and-Wait)
$tcp set maxcwnd_ 1 ; # Define a janela de congestionamento máxima para 1
$tcp set fid_ 1 ; # Associa o Agente TCP (pacotes de dados) ao FID 1 para coloração no NAM

$ns attach-agent $n0 $tcp ; # Anexa o agente TCP ao nó emissor (n0)

# Criação e configuração do Agente TCPSink (Receptor)
set sink [new Agent/TCPSink]
$sink set fid_ 1 ; # Associa o Agente TCPSink (ACKs) ao FID 1 para coloração no NAM
$ns attach-agent $n1 $sink ; # Anexa o agente TCPSink ao nó receptor (n1)

$ns connect $tcp $sink ; # Conecta o agente TCP ao agente TCPSink

# Criação e anexação de uma aplicação (FTP) ao agente TCP
set ftp [new Application/FTP]
$ftp attach-agent $tcp ; # Anexa a aplicação FTP ao agente TCP

# Habilita o rastreamento de variáveis específicas do agente TCP (ex: cwnd_)
$ns add-agent-trace $tcp tcp
$ns monitor-agent-trace $tcp
$tcp tracevar cwnd_ ; # Rastrea a variável cwnd_ (janela de congestionamento)

# Eventos de início e fim da simulação
$ns at 0.1 "$ftp start" ; # Inicia a aplicação FTP em 0.1 segundos
$ns at 3.0 "$ns detach-agent $n0 $tcp ; $ns detach-agent $n1 $sink" ; # Desanexa agentes em 3.0s
$ns at 3.5 "finish" ; # Chama a função finish em 3.5 segundos

# Anotações para a visualização NAM (eventos importantes na simulação)
$ns at 0.0 "$ns trace-annotate \"Stop and Wait - operação normal\""
$ns at 0.05 "$ns trace-annotate \"FTP iniciando em 0.1\""
$ns at 0.11 "$ns trace-annotate \"Enviando Pacote_0\""
$ns at 0.35 "$ns trace-annotate \"Recebendo ACK_0\""
$ns at 0.56 "$ns trace-annotate \"Enviando Pacote_1\""
$ns at 0.79 "$ns trace-annotate \"Recebendo ACK_1\""
$ns at 0.99 "$ns trace-annotate \"Enviando Pacote_2\""
$ns at 1.23 "$ns trace-annotate \"Recebendo ACK_2\""
$ns at 1.43 "$ns trace-annotate \"Enviando Pacote_3\""
$ns at 1.67 "$ns trace-annotate \"Recebendo ACK_3\""
$ns at 1.88 "$ns trace-annotate \"Enviando Pacote_4\""
$ns at 2.11 "$ns trace-annotate \"Recebendo ACK_4\""
$ns at 2.32 "$ns trace-annotate \"Enviando Pacote_5\""
$ns at 2.55 "$ns trace-annotate \"Recebendo ACK_5\""
$ns at 2.75 "$ns trace-annotate \"Enviando Pacote_6\""
$ns at 2.99 "$ns trace-annotate \"Recebendo ACK_6\""
$ns at 3.1 "$ns trace-annotate \"FTP terminou\""

# Procedimento que é chamado ao final da simulação
proc finish {} {
    global ns nf f ; # Declara variáveis globais
    $ns flush-trace ; # Garante que todos os eventos sejam gravados nos arquivos de rastreamento
    close $nf ; # Fecha o arquivo .nam
    close $f ; # Fecha o arquivo .tr

    puts "filtering..." ; # Mensagem para o console
    puts "running nam..." ; # Mensagem para o console
    exec nam 01-stop-n-wait.nam & ; # Executa o NAM com o arquivo gerado
    exit 0 ; # Encerra o script
}

$ns run ; # Inicia a simulação
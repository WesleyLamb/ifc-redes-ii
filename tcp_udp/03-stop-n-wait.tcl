# Simulação do Protocolo Stop-and-Wait em Rede Multicaminho com Tráfego Concorrente
# Este script demonstra o Stop-and-Wait (fluxo TCP/FTP) em uma topologia mais complexa,
# com roteadores intermediários e a presença de tráfego CBR concorrente.
# Autor: Angelita Rettore de Araujo
# Última alteração: 2025-08-19
# versão 1.0

set ns [new Simulator] ; # Cria uma nova instância do simulador

# Define as cores para os FIDs (Flow IDs)
$ns color 1 red ;  # Cor vermelha para o fluxo CBR (FID 1)
$ns color 2 blue ; # Cor azul para o fluxo Stop-and-Wait (FTP) (FID 2)

# Abre os arquivos de rastreamento para o NAM e para análise de texto
set f [open 03-stop-n-wait.tr w] ; # Abre arquivo .tr para rastreamento detalhado
$ns trace-all $f ; # Habilita o rastreamento de todos os eventos para o .tr
set nf [open 03-stop-n-wait.nam w] ; # Abre arquivo .nam para visualização
$ns namtrace-all $nf ; # Habilita o rastreamento de todos os eventos para o NAM


### Função para construir a topologia da rede
proc build_topology { ns } {

        global node_ ; # Declara a variável global para os nós, que armazenará os objetos nó

        # Criação dos nós da rede
        set node_(s1) [$ns node] ; # s1: Emissor FTP (Stop-and-Wait)
        set node_(s2) [$ns node] ; # s2: Emissor CBR
        set node_(r1) [$ns node] ; # r1: Roteador 1
        set node_(r2) [$ns node] ; # r2: Roteador 2
        set node_(s3) [$ns node] ; # s3: Receptor FTP (Stop-and-Wait)
        set node_(s4) [$ns node] ; # s4: Receptor CBR

        # Configuração de cor e forma dos nós 
        $node_(s2) color "red"
        $node_(s4) color "red"
        $node_(s3) color "blue"
        $node_(s1) color "blue"
        $node_(r1) shape "square"
        $node_(r2) shape "square"


        # Rótulos para os nós na visualização NAM (mais específicos para clareza)
        $ns at 0.0 "$node_(s1) label Emissor-FTP"
        $ns at 0.0 "$node_(s2) label Emissor-CBR"
        $ns at 0.0 "$node_(s3) label Receptor-FTP"
        $ns at 0.0 "$node_(s4) label Receptor-CBR"

        # Criação dos links duplex com suas características
        # Largura de banda: 0.5Mb/s, Atraso de propagação: 50ms, Tipo de fila: DropTail
        $ns duplex-link $node_(s1) $node_(r1) 0.5Mb 50ms DropTail
        $ns duplex-link $node_(s2) $node_(r1) 0.5Mb 50ms DropTail
        $ns duplex-link $node_(r1) $node_(r2) 0.5Mb 50ms DropTail
        $ns duplex-link $node_(r2) $node_(s3) 0.5Mb 50ms DropTail
        $ns duplex-link $node_(r2) $node_(s4) 0.5Mb 50ms DropTail

        # Limites de fila para os links intermediários (entre roteadores)
        $ns queue-limit $node_(r1) $node_(r2) 100
        $ns queue-limit $node_(r2) $node_(r1) 100

        # Orientação visual dos links no NAM para organizar o layout
        $ns duplex-link-op $node_(s1) $node_(r1) orient right-down
        $ns duplex-link-op $node_(s2) $node_(r1) orient right-up
        $ns duplex-link-op $node_(r1) $node_(r2) orient right
        $ns duplex-link-op $node_(r2) $node_(s3) orient right-up
        $ns duplex-link-op $node_(r2) $node_(s4) orient right-down

        # Posição da fila no display NAM para links específicos (útil para links congestionados)
        $ns duplex-link-op $node_(r1) $node_(r2) queuePos 0.5
        $ns duplex-link-op $node_(r2) $node_(r1) queuePos 0.5

}

build_topology $ns ; # Chama a função para construir a topologia da rede

Agent/TCP set nam_tracevar_ true ; # Habilita variáveis de rastreamento do TCP para o NAM

### Configuração do protocolo Stop-and-Wait (TCP/FTP) entre s1 e s3 (Fluxo Azul)
set tcp [new Agent/TCP]
$tcp set window_ 1 ; # Define a janela de envio para 1 pacote (característica do Stop-and-Wait)
$tcp set maxcwnd_ 1 ; # Define a janela de congestionamento máxima para 1
$tcp set fid_ 2 ; # Associa este fluxo (pacotes de dados e ACKs) ao FID 2 (azul)

$ns attach-agent $node_(s1) $tcp ; # Anexa o agente TCP ao nó s1 (Emissor FTP)

set sink [new Agent/TCPSink]
$sink set fid_ 2 ; # Associa o TCPSink (para receber dados e enviar ACKs) ao FID 2 (azul)
$ns attach-agent $node_(s3) $sink ; # Anexa o TCPSink ao nó s3 (Receptor FTP)

$ns connect $tcp $sink ; # Conecta o agente TCP ao TCPSink para estabelecer a comunicação


set ftp [new Application/FTP]
$ftp attach-agent $tcp ; # Anexa a aplicação FTP ao agente TCP, gerando o tráfego

$ns add-agent-trace $tcp tcp ; # Adiciona rastreamento para o agente TCP
$ns monitor-agent-trace $tcp ; # Monitora rastreamento do agente TCP
$tcp tracevar cwnd_ ; # Rastrea a variável cwnd_ (janela de congestionamento) para análise

### Configuração do tráfego CBR (Constant Bit Rate) entre s2 e s4 (Fluxo Vermelho - FID 1)
# Utiliza o comando create-connection que simplifica a criação do agente e aplicação
set cbr_app [$ns create-connection CBR $node_(s2) Null $node_(s4) 1] ; # Cria conexão CBR de s2 para s4, com FID 1

### Definição dos eventos da simulação
$ns at 0.1 "$ftp start" ;     # Inicia a aplicação FTP em 0.1 segundos
$ns at 1.7 "$ftp stop" ;      # Interrompe a aplicação FTP em 1.7 segundos
$ns at 0.1 "$cbr_app start" ; # Inicia a aplicação CBR em 0.1 segundos
$ns at 1.7 "$cbr_app stop" ;  # Interrompe a aplicação CBR em 1.7 segundos
$ns at 2.0 "finish" ;         # Chama a função finish para encerrar a simulação em 2.0 segundos

### Anotações para a visualização NAM (eventos importantes e marcadores de tempo)
$ns at 0.0 "$ns trace-annotate \"Stop and Wait com tráfego concorrente (FTP azul, CBR vermelho)\""
$ns at 0.05 "$ns trace-annotate \"FTP e CBR iniciam em 0.1\""
$ns at 0.11 "$ns trace-annotate \"FTP: Enviando Pacote_0\""
$ns at 0.30 "$ns trace-annotate \"FTP: Recebendo Ack_0\""
$ns at 0.45 "$ns trace-annotate \"FTP: Enviando Pacote_1\""
$ns at 0.65 "$ns trace-annotate \"FTP: Recebendo Ack_1\""
$ns at 0.80 "$ns trace-annotate \"FTP: Enviando Pacote_2\""
$ns at 1.00 "$ns trace-annotate \"FTP: Recebendo Ack_2\""
$ns at 1.15 "$ns trace-annotate \"FTP: Enviando Pacote_3\""
$ns at 1.35 "$ns trace-annotate \"FTP: Recebendo Ack_3\""
$ns at 1.50 "$ns trace-annotate \"FTP: Enviando Pacote_4\""
$ns at 1.70 "$ns trace-annotate \"FTP: Recebendo Ack_4\""
$ns at 1.80 "$ns trace-annotate \"FTP e CBR terminaram\""

# Procedimento que é chamado ao final da simulação
proc finish {} {
    global ns nf f ; # Declara as variáveis globais do simulador e arquivos
    $ns flush-trace ; # Garante que todos os eventos pendentes sejam gravados nos arquivos de rastreamento
    close $f ; # Fecha o arquivo .tr (detalhes de texto)
    close $nf ; # Fecha o arquivo .nam (para visualização)

    puts "filtering..." ; # Mensagem de console indicando processamento
    puts "running nam..." ; # Mensagem de console indicando a execução do NAM
    exec nam 03-stop-n-wait.nam & ; # Executa o NAM com o arquivo gerado para visualização
    exit 0 ; # Encerra o script Tcl
}

$ns run ; # Inicia a execução da simulação
# Simulação do Protocolo Stop-and-Wait em Condições de Congestão
# Este script demonstra o comportamento do Stop-and-Wait em uma rede com links congestionados
# (limite de fila reduzido) e a presença de tráfego concorrente (CBR).
# Ilustra a perda de pacotes devido ao congestionamento e o mecanismo de retransmissão do Stop-and-Wait.
# Autor: Angelita Rettore de Araujo
# Última alteração: 2025-08-19
# versão 1.0

set ns [new Simulator] ; # Cria uma nova instância do simulador

# Define as cores para os FIDs (Flow IDs)
$ns color 1 red ;  # Cor vermelha para o fluxo CBR (FID 1)
$ns color 2 blue ; # Cor azul para o fluxo Stop-and-Wait (FTP) (FID 2)

# Abre os arquivos de rastreamento para o NAM e para análise de texto
set f [open 04-stop-n-wait-loss.tr w] ; # Abre arquivo .tr para rastreamento detalhado de eventos
$ns trace-all $f ; # Habilita o rastreamento de todos os eventos para o .tr
set nf [open 04-stop-n-wait-loss.nam w] ; # Abre arquivo .nam para visualização gráfica da simulação
$ns namtrace-all $nf ; # Habilita o rastreamento de todos os eventos para o NAM


### Função para construir a topologia da rede
proc build_topology { ns } {

        global node_ ; # Declara a variável global 'node_', que armazenará os objetos nó criados

        # Criação dos seis nós da rede
        set node_(s1) [$ns node] ; # s1: Nó Emissor para o tráfego FTP (Stop-and-Wait)
        set node_(s2) [$ns node] ; # s2: Nó Emissor para o tráfego CBR
        set node_(r1) [$ns node] ; # r1: Nó Roteador intermediário 1
        set node_(r2) [$ns node] ; # r2: Nó Roteador intermediário 2
        set node_(s3) [$ns node] ; # s3: Nó Receptor para o tráfego FTP (Stop-and-Wait)
        set node_(s4) [$ns node] ; # s4: Nó Receptor para o tráfego CBR

        # Configuração de cor dos nós conforme o tipo de tráfego associado
        $node_(s2) color "red" ;  # Emissor CBR (nó de origem do fluxo vermelho)
        $node_(s4) color "red" ;  # Receptor CBR (nó de destino do fluxo vermelho)

        $node_(s3) color "blue" ; # Receptor Stop-and-Wait (nó de destino do fluxo azul)
        $node_(s1) color "blue" ; # Emissor Stop-and-Wait (nó de origem do fluxo azul)

        # Configuração de forma dos roteadores (quadrada para distinção visual)
        $node_(r1) shape "square"
        $node_(r2) shape "square"

        # Rótulos para os nós na visualização NAM (indicando a função de cada nó)
        $ns at 0.0 "$node_(s1) label Stop&Wait"
        $ns at 0.0 "$node_(s2) label CBR"
        $ns at 0.0 "$node_(s3) label Stop&Wait"
        $ns at 0.0 "$node_(s4) label CBR"

        # Criação dos links duplex entre os nós com suas características
        # Largura de banda: 0.5Mb/s, Atraso de propagação: 50ms, Tipo de fila: DropTail
        $ns duplex-link $node_(s1) $node_(r1) 0.5Mb 50ms DropTail
        $ns duplex-link $node_(s2) $node_(r1) 0.5Mb 50ms DropTail
        $ns duplex-link $node_(r1) $node_(r2) 0.5Mb 50ms DropTail
        $ns duplex-link $node_(r2) $node_(s3) 0.5Mb 50ms DropTail
        $ns duplex-link $node_(r2) $node_(s4) 0.5Mb 50ms DropTail

        # Limites de fila para os links intermediários (entre roteadores), reduzidos para forçar congestionamento
        $ns queue-limit $node_(r1) $node_(r2) 2 ; # Fila de apenas 2 pacotes para r1->r2
        $ns queue-limit $node_(r2) $node_(r1) 2 ; # Fila de apenas 2 pacotes para r2->r1

        # Orientação visual dos links no NAM para organizar o layout da topologia
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

### Configuração do protocolo Stop-and-Wait (TCP/FTP) entre s1 e s3 (Fluxo Azul - FID 2)
set tcp [new Agent/TCP]
$tcp set window_ 1 ; # Define a janela de envio para 1 pacote (característica do Stop-and-Wait)
$tcp set maxcwnd_ 1 ; # Define a janela de congestionamento máxima para 1
$ns attach-agent $node_(s1) $tcp ; # Anexa o agente TCP ao nó s1 (Emissor Stop-and-Wait)
$tcp set fid_ 2 ; # Associa este fluxo (pacotes de dados e ACKs) ao FID 2 (azul)

set sink [new Agent/TCPSink]
$ns attach-agent $node_(s3) $sink ; # Anexa o TCPSink ao nó s3 (Receptor Stop-and-Wait)

$ns connect $tcp $sink ; # Conecta o agente TCP ao TCPSink para estabelecer a comunicação

set ftp [new Application/FTP]
$ftp attach-agent $tcp ; # Anexa a aplicação FTP ao agente TCP, gerando o tráfego

$ns add-agent-trace $tcp tcp ; # Adiciona rastreamento para o agente TCP
$ns monitor-agent-trace $tcp ; # Monitora rastreamento do agente TCP
$tcp tracevar cwnd_ ; # Rastrea a variável cwnd_ (janela de congestionamento) para análise

### Configuração do tráfego CBR (Constant Bit Rate) entre s2 e s4 (Fluxo Vermelho - FID 1)
# Utiliza o comando create-connection que simplifica a criação do agente e aplicação
set cbr [$ns create-connection CBR $node_(s2) Null $node_(s4) 1] ; # Cria conexão CBR de s2 para s4, com FID 1


### Definição dos eventos da simulação no tempo
$ns at 0.1 "$ftp start" ;     # Inicia a aplicação FTP em 0.1 segundos
$ns at 2.35 "$ftp stop" ;     # Interrompe a aplicação FTP em 2.35 segundos
$ns at 0.1 "$cbr start" ;     # Inicia a aplicação CBR em 0.1 segundos
$ns at 2.35 "$cbr stop" ;     # Interrompe a aplicação CBR em 2.35 segundos
# $ns at 0.48 "$ns queue-limit $node_(r1) $node_(r2) 1" ; # Linha comentada no original, possivelmente para simular mais perda
$ns at 0.52 "$ns queue-limit $node_(r1) $node_(r2) 2" ; # Restaura/mantém o limite da fila em 2 em 0.52s
$ns at 3.0 "finish" ;         # Chama a função finish para encerrar a simulação em 3.0 segundos

### Anotações para a visualização NAM (eventos importantes e marcadores de tempo)
$ns at 0.0 "$ns trace-annotate \"Stop and Wait com perda de pacotes (devido a congestionamento)\""
$ns at 0.05 "$ns trace-annotate \"FTP e CBR iniciam em 0.1\""
$ns at 0.11 "$ns trace-annotate \"Enviando pacote_0 (FTP)\""
$ns at 0.30 "$ns trace-annotate \"Recebendo Ack_0 (FTP)\""
$ns at 0.45 "$ns trace-annotate \"Enviando pacote_1 (FTP)\""
$ns at 0.50 "$ns trace-annotate \"Pacote_1 perdido (devido à fila cheia)\""
$ns at 0.55 "$ns trace-annotate \"Aguardando Ack_1 (FTP)\""
$ns at 1.34 "$ns trace-annotate \"Timeout -> retransmitindo pacote_1 (FTP)\""
$ns at 1.55 "$ns trace-annotate \"Recebendo Ack_1 (FTP)\""
$ns at 1.70 "$ns trace-annotate \"Enviando pacote_2 (FTP)\""
$ns at 1.90 "$ns trace-annotate \"Recebendo Ack_2 (FTP)\""
$ns at 2.0 "$ns trace-annotate \"FTP e CBR terminaram\""

# Procedimento que é chamado ao final da simulação
proc finish {} {
    global ns nf f ; # Declara as variáveis globais do simulador e arquivos de rastreamento
    $ns flush-trace ; # Garante que todos os eventos pendentes sejam gravados nos arquivos de rastreamento
    close $f ; # Fecha o arquivo .tr (detalhes de texto)
    close $nf ; # Fecha o arquivo .nam (para visualização no NAM)

    puts "filtering..." ; # Mensagem de console indicando processamento
    puts "running nam..." ; # Mensagem de console indicando a execução do NAM
    exec nam 04-stop-n-wait-loss.nam & ; # Executa o NAM com o arquivo gerado para visualização
    exit 0 ; # Encerra o script Tcl com sucesso
}

$ns run ; # Inicia a execução da simulação
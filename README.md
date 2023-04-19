# Implementação de um Simulador de Cache Parametrizável em C

## Info
Nome: Otávio Salomão Rocha -- M2 <br/>
Discord: Lekaso#5775 <br/>
Email: otavio.sr@inf.ufpel.edu.br

## Especificações
   Implementação de um simulador de caches, podendo simular diferentes tamanhos, tamanhos de bloco e associatividade, onde os parâmetros são passado por linha de comando.

   O programa tem somente a implementação da política de substituição Random.

   Para compilar foi utilizado a seguinte linha de comando:
    
   <pre><code> gcc main.c -o cache_simulator -Wall -lm </code></pre>
    
   Como foi utilizada a biblioteca *math.h* se faz necessário o *-lm* do final para que a compilação seja feita com sucesso.

   Na execução, siga o padrão: **cache_simulator nsets bsize assoc substituição flag_saida arquivo_de_entrada**

   Por exemplo: <pre><code> ./cache_simulator 256 4 1 R 1 bin_100.bin </code></pre>

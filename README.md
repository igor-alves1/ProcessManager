# ProcessManager


## Descrição
Projeto realizado durante a disciplina **Sistemas Operacionais** para simular o comportamento de escalonamento de processos para execução em ambiente multi-processado. O projeto simula um sistema computacional com 4 CPUs e 32GB de memória RAM e a política de escalonamento adotada é Round-Robin com quantum de 4 u.t. para o escalonamento de curto-prazo. Não foi implementado um escalonador de médio prazo, logo processos não podem ser suspensos e um processo deve estar totalmente presente em memória principal para ser executado. 

## Como executar
As instruções de compilação e execução foram pensadas para sistemas Linux e testadas no Ubuntu LTS 22. Também é necessário ter instalada a biblioteca GTK4, que pode ser instalada pelo comando ```apt install libgtk-4-dev```.

## Créditos
Feito por [Igor Rodrigues](https://github.com/igor-alves1), [Gabriel Campos](https://github.com/gdac7), [Pedro Castelo Branco](https://github.com/pcbmoreira2), [Ricardo Araújo](https://github.com/rcardoaraujo) e [Júlia Vogel](https://github.com/juliavogelm).

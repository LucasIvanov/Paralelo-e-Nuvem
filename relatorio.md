# Relatório

De início foi recebido um conjunto de códigos seriais, de onde deveriam ser selecionados apenas três e então paralelizados utilizando dois métodos de paralelização distindos. Os métodos de paralelização vieram de conjunto de três métodos no qual deveriamos escolher apenas dois.
Os algoritmos escolhidos para a paralelização foram:
- BucketSort;
- friendly;
- nbody.

Os métodos de paralelização foram:
- OpenMP;
- OpenACC.

## BucketSort
No arquivo fornecido pelo professor a ordenação ocorre da seguinte forma serial:
- Os baldes são criados inicializados e os valores distribuidos para cada _Bucket_.
- A ordenação ocorre em cada _Bucket_ um em seguida do outro.
  - Usando o método do _Insertion Sort_.

Para deixar que a ordenação ocorrece de forma paralelizada, foi utilizado a biblioteca _OpenMP_, que disponibiliza alguns ```pragmas``` para serem utilizados. Com isso foi feita da seguinte maneira a paralelização.
- Com a inserção da biblioteca _OpenMP_, foi apenas inserido antes do _looping for_ que realizava a chamada do _Insertion Sorte_ para cada _Bucket_ um diretiva ```#pragma omp parallel for```
Essa diretiva indica ao processador, que cada iteração do looping deve ser executava em um _threading_ separada.
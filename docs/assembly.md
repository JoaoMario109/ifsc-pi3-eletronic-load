# Montagem da placa da carga

## Introdução

<<Introdução sobre a montagem da placa da carga.>>

## Metodologia de Montagem

Aqui fala um pouco que foi soldado e tal, usando o ibom do kicad para se guiar na solda dos componentes. e adiciona as fotos da gente soldando

## Fotos da Montagem

## Testes

### Verificação da montagem

Primeiramente foram soldados todos os componentes na placa, mas os jumpers mativemos desligados.

Depois a placa foi alimentada e verificado se as tensões estavam corretas, ai fuimos ligando os jumpers de alimentação em sequencia input, 5V, 3v3. 

Depois de conferir que os reguladores funcionaram corretamente foi ligado o microcontrolador e verificado se o mesmo estava funcionando corretamente.

Depois foi ligado o sensor de corrente e verificado se o mesmo estava funcionando corretamente.

Depois foi ligado o ADC e DAC e verificado se o mesmo estava funcionando corretamente.

### Tuning do PID analogico

Foram adicionados trimpots nos resistores do PID para ajustar os ganhos do PID, foi feito um ajuste manual para que o PID funcionasse corretamente.

<Adiciona as fotos do osciloscopio>

### Teste de linearização do ADC

Com o controle analogico funcionando foi feito uma barredura em steps de 10mv no DAC e foi medido com um alicate amperimetro a corrente que a carga exigia e o valor do ADC
com esses valores foi feito uma regresão linear para ajustar o valor do ADC para a corrente real. Ficando com uma precisão de 3%.


## Modificaoes na Placa

- O sensor de corrente na lib do kicad estava ao contrário, então foi necessario passar um fio para inverter o sentido e cortar a trilkha (tem foto no grupo)

- Trilha de enable do rele desligada foi colocado um fio para ligar o enable do rele entre o micro e o transistor de drive (fio amarelo que atravessa a placa)

- Resistores de estabilizaçao do transistor foram auymentados um pouco para aumentar a estabilidade emm troca de aumentar um pouco a tensão minima de operação  (resistores verdes na placa)

- Temporariamente foram utilizados transistores 2n3773 para a parte de potencia pois não tinhamos ainda os transistores projetados, mas para baixa potencia funcionam similar

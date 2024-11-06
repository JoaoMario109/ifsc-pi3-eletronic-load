# Seção Analógica

## Correção de Offset

Os diodos na seção de correção de offset têm a função de compensar o fato de que o amplificador LM324 não é do tipo rail-to-rail, ou seja, ele não consegue atingir os 12V necessários para despolarizar completamente o transistor PNP Q2. Assim, foram utilizados diodos em série para que a queda de tensão neles crie um offset no emissor, garantindo uma operação correta.

## Controle Analógico

O controle analógico da carga eletrônica é composto por duas etapas principais:

### 1. Inversor de Sinal

Esta etapa consiste em um inversor para compensar o fato de que o driver com transistor PNP inverte o sinal. Pela natureza do circuito, com 12V não haverá passagem de corrente pelo transistor PNP, enquanto com 0V haverá a corrente máxima. Após a inversão, com 0V não haverá corrente, e o aumento da corrente será proporcional à tensão de entrada.

### 2. Controlador PID Analógico

A segunda etapa é um controlador PID analógico feito com um amplificador operacional (ampop). Esse controlador é interessante porque, caso seja necessário desativá-lo para utilizar apenas o controle digital, basta colocar um curto em R14 e C6.

O cálculo do controlador foi feito através de um método empírico de sucessivas aproximações, onde, utilizando o LTSpice, foram ajustados os valores de capacitância e resistência até atingir o comportamento desejado. É importante ressaltar que sempre há uma diferença entre simulação e prática; portanto, quando implementado em bancada, será realizado um ajuste fino dos componentes.

| Componente       | Descrição                                                                                                        |
|------------------|------------------------------------------------------------------------------------------------------------------|
| **Capacitor C3** | Aumentar a capacitância de C3 reduz o tempo de subida (rise time), mas também aumenta o fenômeno de ringing.    |
| **Resistor R10** | Reduzir a resistência de R10 aumenta a velocidade do circuito, mas também eleva o ringing.                      |
| **Capacitor C6** | Controla a velocidade de resposta do controlador. Quanto maior a capacitância, mais lenta será a resposta.       |

Os resistores em série com os capacitores têm uma função equivalente à variação direta da capacitância dos capacitores associados.

### Valores Encontrados

| Componente | Valor   |
|------------|---------|
| R14        | 330Ω    |
| C6         | 10nF    |
| R10        | 720Ω    |
| C3         | 100nF   |
| R13        | 5kΩ     |
| R11        | 1kΩ     |

### Simulação

<div style="text-align: center;">
	<img src="./assets/analog-simulation.png" alt="Simulação do Controlador PID Analógico" width="700"/>
</div>

<div style="text-align: center;">
	<img src="./assets/analog-simulation-plot.png" alt="Simulação do Controlador PID Analógico Plot" width="700"/>
</div>

### Observações

A resposta do controlador PID analógico é satisfatória para o projeto, tendo um leve overshoot e settling time aceitável. Ainda assim, é importante ressaltar que a implementação prática pode apresentar diferenças em relação à simulação, e ajustes finos podem ser necessários.

---

## Sensor de Tensão

A configuração do sensor de tensão consiste em um divisor de tensão simétrico com redução de modo comum, conectado a um amplificador operacional em modo diferencial.

### Divisor de Tensão

O divisor de tensão é composto por três etapas acopladas:

1. **Divisor de Tensão de Modo Comum**: Resistores conectados ao GND, em conjunto com resistores em série com a entrada, atuam como divisor de tensão para o modo comum.
2. **Divisor de Tensão Diferencial**: Um resistor que conecta os resistores do divisor em série com a entrada funciona como um divisor diferencial, configurado de forma simétrica para maximizar a redução de ruído.

Para o cálculo dos valores desses divisores, foi utilizado o método empírico de sucessivas aproximações.

### Amplificador Operacional em Modo Diferencial

O amplificador operacional diferencial é usado nesta configuração porque a entrada terá um offset de tensão devido às quedas de tensão nos fios e trilhas. O ganho foi ajustado para que, com uma entrada de 120V no divisor de tensão, a saída forneça 3.3V, aproveitando todo o range do ADC.

### Filtros

Existem três etapas de filtro passa-baixa no circuito:

- **Entrada (2 filtros)**: Reduzem o ruído de modo comum e diferencial.
- **Saída (1 filtro)**: Reduz o ruído de saída do amplificador operacional.

Todos os filtros foram calculados para ter uma frequência de corte aproximada de 1kHz.

### Proteções

O circuito conta com proteções contra sobretensão e tensão reversa, tanto na entrada quanto na saída. A proteção de sobretensão é feita por diodos TVS.

### Valores Encontrados

| Componente | Valor   |
|------------|---------|
| R5, R6     | 50kΩ    |
| R7, R8, R9 | 1kΩ     |
| C2, C4     | 100nF   |
| C3         | 10nF    |
| R10        | 500Ω    |
| C1         | 1uF     |

### Simulação

<div style="text-align: center;">
	<img src="./assets/voltage-sensor-simulation.png" alt="Simulação do Sensor de Tensão" width="700"/>
</div>

<div style="text-align: center;">
	<img src="./assets/voltage-sensor-simulation-plot.png" alt="Simulação do Sensor de Tensão Plot" width="700"/>
</div>

---

# Seção de Potência

## Polarização do Transistor de Potência e Driver

Para determinar os valores dos resistores de base dos transistores de potência, foi utilizado o método de sucessivas aproximações, buscando um ponto de operação onde o transistor tenha corrente de base suficiente para entrar em saturação.

### Valores Encontrados

| Componente               | Valor |
|--------------------------|-------|
| Resistor da base         | 20Ω   |
| Resistor da base do driver | 2.2kΩ |
| Resistor do emissor      | 0.1Ω  |

---

## Cálculo da Resistência Térmica Máxima do Dissipador

Para determinar a resistência térmica máxima do dissipador necessário para dissipar uma potência total de 100W distribuída entre dois transistores, com aplicação direta de pasta térmica, foram consideradas as seguintes especificações:

| Especificação                        | Valor    |
|--------------------------------------|----------|
| Resistência térmica junction-to-case (Rθjc) | 1.25 °C/W |
| Temperatura ambiente máxima          | 30 °C    |
| Temperatura máxima de operação       | 120 °C   |

### Cálculo

1. **Cálculo do aumento de temperatura permitido ($\Delta T$)**:
   $$
   \Delta T = T_{max} - T_{amb} = 120 \,°C - 30 \,°C = 90 \,°C
   $$

2. **Distribuição da Potência**: Cada transistor dissipará 50W.

3. **Resistência Térmica Junction-to-Case Total**:
   $$
   R_{\theta jc} \times P = 1.25 \,°C/W \times 50 \,W = 62.5 \,°C
   $$

4. **Resistência Térmica Necessária para o Dissipador ($R_{\theta sa}$)**:
   $$
   R_{\theta sa} = \frac{\Delta T - (R_{\theta jc} \times P)}{P_{total}} = \frac{27.5 \,°C}{100 \,W} = 0.275 \,°C/W
   $$

Portanto, para manter a temperatura dos transistores abaixo de 120 °C, a resistência térmica máxima do dissipador deve ser **0.275 °C/W**.

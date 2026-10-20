# Vending Machine

| Supported Targets | ESP32 | ESP32-C2 | ESP32-C3 | ESP32-C6 | ESP32-H2 | ESP32-P4 | ESP32-S2 | ESP32-S3 |
| ----------------- | ----- | -------- | -------- | -------- | -------- | -------- | -------- | -------- |
|                   |   YES |          |          |          |          |          |          |          |

Projeto de automação de uma máquina de vendas (Vending Machine), desenvolvido como **Projeto Integrador III** do Curso Superior de Tecnologia em Sistemas Embarcados da **FATEC Santo André**.

---

##  Sobre o Projeto

O sistema gerencia a seleção de produtos, navegação de menus via display e acionamento de dispensadores mecânicos, utilizando o framework oficial **ESP-IDF**.

### Principais Funcionalidades
- Navegação de menu interativa em display LCD.
- Controle de motores de passo para liberação do produto.
- Sistema de debouncing por software para navegação segura nos botões.

---

##  Hardware Utilizado

- **Microcontrolador:** ESP32-WROOM-32
- **Interface:** Display LCD / TFT com driver dedicado
- **Atuador:** Motor de Passo NEMA 17 + Driver A4988
- **Entradas:** Botões de navegação e seleção

---
<div id="top"></div>

<!-- PROJECT LOGO -->
<br />
<div align="center">
  <a href="https://github.com/JoaoMario109/ifsc-pi3-eletronic-load">
    <img src="./docs/logo.png" alt="Project Logo" width="80" height="80">
  </a>

  <h3 align="center">Simple Electronic Load</h3>

  <p align="center">
    <br />
    <br />
    <a href="https://github.com/JoaoMario109/ifsc-pi3-eletronic-load/issues">Report Bug</a>
    ·
    <a href="https://github.com/JoaoMario109/ifsc-pi3-eletronic-load/issues">Request Feature</a>
  </p>
</div>


<!-- TABLE OF CONTENTS -->
<details>
  <summary>Table of Contents</summary>
  <ol>
    <li>
      <a href="#about-the-project">About The Project</a>
      <ul>
        <li><a href="#about-the-project-overview">Overview</a></li>
      </ul>
    </li>
    <li>
      <a href="#license">License</a>
    </li>
    <li>
      <a href="#contact">Contacts</a>
    </li>
  </ol>
</details>


<!-- ABOUT THE PROJECT -->
<div id="about-the-project"></div>

## About The Project

This project is the implementation of a simple electronic load with WiFi integration and the ability to operate using custom waveforms. It forms part of (PI3) for the Electronics Engineering course at IFSC, Campus Florianópolis.

<div id="about-the-project-overview"></div>

### Overview

The electronic load is designed to handle up to 100W of power and supports four different operating modes: constant current, constant power, constant resistance, and constant voltage. Additionally, it can execute custom waveforms uploaded via its various communication interfaces.

The load is divided into three key modules:
1. **Power Module** – Contains the hardware components responsible for handling the load, such as the power transistors and other essential elements.
2. **Control Module** – Manages the desired operating mode and ensures the system maintains the appropriate levels of current, voltage, or resistance.
3. **Communication Module** – Facilitates interaction between the user and the device via WiFi, USB, or other interfaces, and stores custom waveforms on an SD card for future use.

The device can be accessed through its control panel, WiFi, or USB interface, and its communication capabilities can easily be expanded. Any compatible interface can be connected to the main I2C bus using an SCPI-like protocol.

<p align="right">(<a href="#top">back to top</a>)</p>

<!-- LICENSE -->
<div id="license"></div>

## License

Distributed under the MIT License. See [LICENSE](LICENSE) for more information.

<p align="right">(<a href="#top">back to top</a>)</p>

<hr />

<!-- CONTACT -->
<div id="contact"></div>

## Contact

Alejo Perdomo Milar [alejo.pm@aluno.ifsc.edu.br](alejo.pm@aluno.ifsc.edu.br)\
João Mário Carnieletto Izoton Lago [joao.mcil2003@aluno.ifsc.edu.br](joao.mcil2003@aluno.ifsc.edu.br)\
Paulo Cezar Ventura Junior [paulo.cvj@aluno.ifsc.edu.br](paulo.cvj@aluno.ifsc.edu.br)

Project Link: [https://github.com/JoaoMario109/ifsc-pi3-eletronic-load](https://github.com/JoaoMario109/ifsc-pi3-eletronic-load)

<p align="right">(<a href="#top">back to top</a>)</p>

// Stepper motor controller

#include <SPI.h>
#include <Ethernet.h>


// Potenciômetro = A0

// Botões = D02 e D03

// LEDs = D06, D07, D08, D09



// Defines constants

#define MODO_MAX 3

#define ATRASO_MAX 1000

#define ATRASO_MIN 50

#define PASSOS_MOTOR 200



// Motor

int pinosMotor[] = {6, 7, 8}; // Digital

int estadoMotor[] = {0, 0, 0};

int faseAtual = 0;

int velocidade;

float velocidadeRPM = 0;

int modoOperacao = 0;

char sentido = 'H';



int passoAtual = 0;

int passoDestino;



int ordemPassoCompleto[3][3] = {{1, 0, 0},

						  		{0, 1, 0},

						  		{0, 0, 1}};



int ordemMeioPasso[6][3] = {{1, 0, 0},

						  	{1, 1, 0},

						  	{0, 1, 0},

						  	{0, 1, 1},

						  	{0, 0, 1},

						  	{1, 0, 1}};



// Buttons

int botaoModo = 2; // Digital

int botaoSentido = 3; // Digital



// Potentiometer

int pinoPot = 0; // Analog

int valorPot;



// Aux`s

int i;

// Web Server

byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
IPAddress ip(192, 168, 5, 25);

EthernetServer server(5005);


// Declaring the functions

void trocaModo();

void trocaSentido();

void leituraPot();

void passoCompleto();

void meioPasso();

void meioPasso();

void controleAngular();

void calculaVelocidade();

void enviaDados();



void setup(){

	// Initiates Serial 

	Serial.begin(9600);

	Ethernet.begin(mac, ip);
        server.begin();
        Serial.print("server is at ");
        Serial.println(Ethernet.localIP());

	// Defines pin type

	pinMode(pinoPot, INPUT);



	// Turns on pull up from buttons

	pinMode(botaoSentido, INPUT_PULLUP);

	pinMode(botaoModo, INPUT_PULLUP);



	// Defines Motor Pins as Output

	for(i = 0; i < 3; i++){

		pinMode(pinosMotor[i], OUTPUT);

	}



	// Initiates interruptions for pins D02 and D03

	attachInterrupt(0, trocaModo, FALLING);

	attachInterrupt(1, trocaSentido, FALLING);

}



void loop(){

	// Function to read and convert value from potentiometer

	leituraPot();



	// Modus Operandi Motor

	switch(modoOperacao){

	    case 0:

	      // Motor stopped

	      break;

	    case 1:

	      // Full Step

	      passoCompleto();

	      delay(velocidade);

	      break;

	    case 2:

	      // Half Step

	      meioPasso();

	      delay(velocidade / 2);

	      break;

	    case 3:

	      // Angle Control

	      controleAngular();

	      break;

	    default:

	    	modoOperacao = 0;

	}



	// Writes the corresponding value to each motor pin

	for(i = 0; i < 3; i++){

		digitalWrite(pinosMotor[i], estadoMotor[i]);

	}



	// Output via TCP protocol

	enviaDados();

}



// Function to change operation mode

void trocaModo(){

	delay(50);

	modoOperacao++;

	if(modoOperacao > MODO_MAX)

		modoOperacao = 0;

}



// Function to reverse rotation

void trocaSentido(){

	delay(50);

	if(sentido == 'A'){

		sentido = 'H';

	}else{

		sentido = 'A';

	}

}



// Function to read Potentiometer value

void leituraPot(){

	valorPot = analogRead(pinoPot);



	passoDestino = map(valorPot, 0, 1023, 0, PASSOS_MOTOR);



	velocidade = map(valorPot, 0, 1023, ATRASO_MAX, ATRASO_MIN);

}



// Full Step Function

void passoCompleto(){

	if(sentido == 'H')

		faseAtual++;



	if(sentido == 'A')

		faseAtual--;



	if(faseAtual > 2)

		faseAtual = 0;



	if(faseAtual < 0)

		faseAtual = 2;



	for(i = 0; i < 3; i++){

		estadoMotor[i] = ordemPassoCompleto[faseAtual][i];

	}

}



// Half Step Function

void meioPasso(){

	if(sentido == 'H')

		faseAtual++;



	if(sentido == 'A')

		faseAtual--;



	if(faseAtual > 5)

		faseAtual = 0;



	if(faseAtual < 0)

		faseAtual = 5;



	for(i = 0; i < 3; i++){

		estadoMotor[i] = ordemMeioPasso[faseAtual][i];

	}

}



void controleAngular(){

	if(passoAtual < passoDestino){

		passoAtual++;

		faseAtual++;

	}



	if(passoAtual > passoDestino){

		passoAtual--;

		faseAtual--;

	}


// Angle Control Function

	if(sentido == 'A'){

		if(faseAtual > 5)

			faseAtual = 0;



		if(faseAtual < 0)

			faseAtual = 5;



		for(i = 0; i < 3; i++){

			estadoMotor[i] = ordemMeioPasso[faseAtual][i];

		}

	}



	if(sentido == 'H'){

		if(faseAtual > 2)

			faseAtual = 0;



		if(faseAtual < 0)

			faseAtual = 2;



		for(i = 0; i < 3; i++){

			estadoMotor[i] = ordemPassoCompleto[faseAtual][i];

		}

	}



	delay(ATRASO_MIN);

}



void calculaVelocidade(){
    
  
       
	velocidadeRPM = 60 / (PASSOS_MOTOR * velocidade) / 1000;

}

	

// Send Serial Data

void enviaDados(){
  
  calculaVelocidade();

  
  EthernetClient client = server.available();
  if (client) {
    Serial.println("new client");
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);

        if (c == '\n' && currentLineIsBlank) {
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");
          client.println("Refresh: 3");
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
            client.println("<center>");
              client.println("<br /><br />");

	// Sends direction

	switch(sentido){

		case 'A':

		client.print("Sentido: Horario"); // Sentido horário
                client.println("<br /><br />");
		  break;

		case 'H':

		client.print("Sentido: Anti-Horario"); // Sentido anti-horário
                client.println("<br /><br />");		 

		  break;

	 } 



	// Sends speed in RPM
        client.print("Velocidade do Motor ");        
        client.print(velocidade);
        client.print(" RPM "); 
        client.println("<br /><br />");


	// Sends operation mode

	switch(modoOperacao){

		case 0:

		  client.print("Motor Parado");  // Stopped

		  break;

		case 1:

		  client.print("Motor Atuando em Passo Completo");  // Full Step

		  break;

		case 2:

	          client.print("Motor Atuando em Meio Passo");  // Half Step

		  break;

		case 3:

	          client.print("Motor esta sendo controlado por angulacao");  // Angular control

		  break;

	 } 
            client.println("</center>");
          client.println("</html>");
          break;
        }
     if (c == '\n') {
 
          currentLineIsBlank = true;
        }
        else if (c != '\r') {

          currentLineIsBlank = false;
        }
      }
    }

    delay(1);

    client.stop();
    Serial.println("client disconnected");
  }

}

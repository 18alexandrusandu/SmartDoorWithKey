
int state, comanda, pinComanda = 14, pinLeft = 9, pinRight = 10;
int going = 4;
int outf = 0;
void setup() {
  Serial.begin(9600);
  state = -1;
  comanda = 0;
  pinMode(pinComanda, INPUT);
  pinMode(pinLeft, OUTPUT);
  pinMode(pinRight, OUTPUT);

  // put your setup code here, to run once:

}

void loop() {
  comanda = digitalRead(pinComanda);
  Serial.print("comanda este: ");
  Serial.print(comanda);

  if (state != comanda)
  {
    state = comanda;
    outf = 0;
    if (state == 1)
    {
      digitalWrite(pinLeft, 1);
      delay(50);
      digitalWrite(pinRight, 0);

    } else
    {
      digitalWrite(pinLeft, 0);
      delay(50);
      digitalWrite(pinRight, 1);

    }

  }

  Serial.print("outf:");
  Serial.println(outf);

  if (outf < going)
    outf += 1;
  else
  {
    digitalWrite(pinLeft, 0);
    delay(50);
    digitalWrite(pinRight, 0);
  }
  delay(50);




  // put your main code here, to run repeatedly:

}

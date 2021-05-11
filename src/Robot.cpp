//
// Created by ricky on 19.04.21.
//

#include "Robot.h"
#include <fstream>
using std::cout;


//void Robot::RampGenerator(TWheel &aWheel) {
//    std::stringstream stream;
//    while (thread_wheels) {
//
//            if ((aWheel.set_speed - aWheel.actual_speed) > 0) {
//                aWheel.actual_speed += aWheel.acceleration;
//            } else if ((aWheel.set_speed - aWheel.actual_speed) < 0){
//                aWheel.actual_speed -= aWheel.acceleration;
//            }
//#ifdef DEBUG
//            std::cout << aWheel.actual_speed << std::endl;
//#endif
//            stream << aWheel.command << "," << aWheel.actual_speed << std::flush;
//            push_message(stream.str());
//            stream.str("");
//           // std::cout << "Chci poslat: " << aWheel.command << "," << aWheel.actual_speed << std::endl;
//            sleep_for(500us);
//
//    }
//}

void Robot::RampGenerator() {
    std::stringstream rstream, lstream;
    while (thread_wheels) {

        if ((iWRight.set_speed - iWRight.actual_speed) > 0) {
            iWRight.actual_speed += iWRight.acceleration;
        } else if ((iWRight.set_speed - iWRight.actual_speed) < 0){
            iWRight.actual_speed -= iWRight.acceleration;
        }

        if ((iWLeft.set_speed - iWLeft.actual_speed) > 0) {
            iWLeft.actual_speed += iWLeft.acceleration;
        } else if ((iWLeft.set_speed - iWLeft.actual_speed) < 0){
            iWLeft.actual_speed -= iWLeft.acceleration;
        }
#ifdef DEBUG
        std::cout << aWheel.actual_speed << std::endl;
#endif
        rstream << iWRight.command << "," << iWRight.actual_speed << std::flush;
        lstream << iWLeft.command << "," << iWLeft.actual_speed << std::flush;
        push_message(rstream.str());
        push_message(lstream.str());
        rstream.str("");
        lstream.str("");
        sleep_for(10ms);
    }
}
//TODO: udělat jednu funkci pro setnutí obou kol naráz
void Robot::SetSpeed(Side aSide, float aSpeed) {
    if (aSide == 0){
        iWLeft.set_speed = aSpeed;
    }
    else {
        iWRight.set_speed = aSpeed;
    }
}

void Robot::Info(){
    std::stringstream stream;
    while(thread_information){

        stream << "LODO" << ","<< 1 << std::flush;
        push_message(stream.str());
        stream.str("");

        stream << "RODO" << ","<< 1 << std::flush;
        push_message(stream.str());
        stream.str("");

        for(size_t i = 0; i<3;++i) {
            stream << "SENSOR" << "," << i << std::flush;
            push_message(stream.str());
            stream.str("");
        }
        ros::spinOnce();
        sleep_for(10ms);
       // ros::spinOnce();
    }


}

void Robot::Message_proccesing() {
    std::regex rgx("(\\w*),(\\d+),?(\\d*.\\d*)?");
    std::smatch matches;
    uint8_t zero_distancer = 0;
    uint8_t zero_distancel = 0;
    while (thread_zpracovavac) {
        if(!empty_box()) {
            std::string message = get_message();
            if (std::regex_search(message, matches, rgx)) {
                if (matches[1].str() == "LODO") {
                    int distance = stoi(matches[2]);

                    if (distance == 0 && iWLeft.actual_speed > 500) {
                        ++zero_distancel;
                    } else {
                        LODO += distance;
                        zero_distancel=0;
                    }
                }
                else if (matches[1].str() == "RODO") {
                    int distance = stoi(matches[2]);
                    if (distance == 0 && iWRight.actual_speed > 500) {
                        ++zero_distancer;
                    }
                    else {
                        RODO += distance;
                        zero_distancer=0;
                    }
                } else if (matches[1].str() == "SENSOR") {
                    sensor[stoi(matches[2])] = std::stof(matches[3]);
                }
            }
            if ((zero_distancel > 10) || (zero_distancer > 10)){
                iWRight.actual_speed=0;
                iWLeft.actual_speed=0;
                zero_distancer=0;
                zero_distancel=0;
            }
        }
        //sleep_for(10us);
    }
}

void Robot::Regulor() {
//TODO:Přidat do regulátoru funkci, že když kola se seknou a znovu začneme přidávat, tak aby regulátor počítal s tím, že jedeme zas od nuly a nedával nesmyslně velkou rychlost

    /*std::fstream file;
    std::fstream file1;
    std::fstream file2;
    file.open("odchylka_rovne.txt",std::ios::out);
    file1.open("pravy_motor_rovne.txt",std::ios::out);
    file2.open("levy_motor_rovne.txt",std::ios::out);*/

    //regulator
#define Kp 8710//9705 //220
#define Ki 0.9 //15.1//6.9//1.3
#define Kd 70//46843//2300

    int magic = 100; //100

    int error = 0;
    int lasterror = 0;
    int integral = 0;
    int derivate = 0;
    int offsetS = 50; //odchylka senzoru

    //ostatni promenne
    int stav = 0;
    int MIN0=100;
    int MAX0=0;
    int MIN1=100;
    int MAX1=0;
    int MIN2=100;
    int MAX2=0;
    int MIN3=100;
    int MAX3=0;
    int s1_hodnota = 0;
    int s0_hodnota = 0;
    int s2_hodnota = 0;
    int s3_hodnota = 0;

    int turn = 50; //otočka
    int offset = 3200; //výchozí rychlost motoru
    int offset_plus = 0;
    int PowerR = 0;
    int PowerL = 0;
    int PowerR_c = 0;
    int PowerL_c = 0;
    int PowerR_Space[6] = {0,0,0,0,0,0};
    int PowerL_Space[6] = {0,0,0,0,0,0};
    int PowerR_SV[20] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    int PowerL_SV[20] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    int Error_SV[20] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    int PowerR_Save = 0;
    int PowerL_Save = 0;
    int i;

    bool flag_calib = 1;
    int calib_pocet = 0;
    int rodo_prev = 0;
    int lodo_prev = 0;
    int error_prum = 0;
    int power_prum = 0;



    while(thread_regulace) {
        //sensory
        if(flag_calib){
            if(MAX1<=sensor[1])
                MAX1=sensor[1];

            if(MIN1>=sensor[1])
                MIN1=sensor[1];

            if(MAX0<=sensor[0])
                MAX0=sensor[0];

            if(MIN0>=sensor[0])
                MIN0=sensor[0];

            if(MAX2<=sensor[2])
                MAX2=sensor[2];

            if(MIN2>=sensor[2])
                MIN2=sensor[2];
            if(MAX3<=sensor[3])
                MAX3=sensor[3];

            if(MIN3>=sensor[3])
                MIN3=sensor[3];
        }

        s0_hodnota=(100-0)*((sensor[0]-MIN0)/(MAX0-MIN0));
        s1_hodnota=(100-0)*((sensor[1]-MIN1)/(MAX1-MIN1));
        s2_hodnota=(100-0)*((sensor[2]-MIN2)/(MAX2-MIN2));
        s3_hodnota=(100-0)*((sensor[3]-MIN3)/(MAX3-MIN3));

        if (s0_hodnota < 4){
            s0_hodnota = 0;
        }
        if (s1_hodnota < 4){
            s1_hodnota = 0;
        }
        if (s2_hodnota < 4){
            s2_hodnota = 0;
        }


        //regulace
        error = offsetS - s1_hodnota;
        integral = integral + error;
        //derivate = error - s1_hodnota;
        derivate = error - lasterror;
        turn = (Kp*error) + (Ki*integral) + (Kd*derivate);
        turn = turn/magic; //rychlost 400 dělono 1000 //rychlost 800 děleno 500 //1600 250 //3200 125 //funkcni 70 na 3200




        switch (stav){

            case 0: //kalibrace senzorů
                SetSpeed(Right, 1000);
                SetSpeed(Left, -1000);

                if (sensor[0] > 3000){
                    flag_calib = 1;
                    stav = 1;
                }
                break;

            case 2: //kalibrace senzorů
                SetSpeed(Right, 1000);
                SetSpeed(Left, -1000);

                //cout << s1_hodnota << "\n";
                //cout << sensor[2] << "\n";
                if (sensor[0] > 3000){
                    lodo_prev = LODO;
                    stav = 4;
                }
                /*if ((RODO-rodo_prev)>1000){
                    lodo_prev = LODO;
                    stav = 1;
                }*/
                break;
            case 1:
                SetSpeed(Right, -1000);
                SetSpeed(Left, 1000);

                if (calib_pocet > 1){ //180 1 270 2
                    stav = 3;
                }
                else {
                    /*if ((LODO-lodo_prev)>1000){
                        rodo_prev = RODO;
                        calib_pocet = calib_pocet + 1;
                        stav = 2;
                    }*/
                    if (sensor[2] > 3000){
                        rodo_prev = RODO;
                        calib_pocet = calib_pocet + 1;
                        stav = 5;
                    }
                }
                break;

            case 4: //kalibrace senzorů
                SetSpeed(Right, 200);
                SetSpeed(Left, -200);

                //cout << s1_hodnota << "\n";
                //cout << sensor[2] << "\n";
                if (sensor[0] < 1500){
                    lodo_prev = LODO;
                    stav = 1;
                }
                /*if ((RODO-rodo_prev)>1000){
                    lodo_prev = LODO;
                    stav = 1;
                }*/
                break;

            case 5:
                SetSpeed(Right, -200);
                SetSpeed(Left, 200);

                /*if ((LODO-lodo_prev)>1000){
                    rodo_prev = RODO;
                    calib_pocet = calib_pocet + 1;
                    stav = 2;
                }*/
                if (sensor[2] < 1500){
                    rodo_prev = RODO;
                    calib_pocet = calib_pocet + 1;
                    stav = 2;
                }
                break;

            case 3:
                flag_calib = 0;
                SetSpeed(Right, -200);
                SetSpeed(Left, 200);

                if ((s1_hodnota<(offsetS+3))&&(s1_hodnota>(offsetS-3))){
                    cout << "START" << "\n";
                    error = 0;
                    lasterror = 0;
                    integral = 0;
                    derivate = 0;
                    stav = 10;
                    rodo_prev = RODO;
                }

                break;

//regulace
            case 10:

                PowerR=offset-turn;
                PowerL=offset+turn;



                SetSpeed(Right, PowerR);
                SetSpeed(Left, PowerL);

                if((RODO-rodo_prev)>100){
                    for (int j = 20; j > 1; --j) {
                        PowerR_SV[j] = PowerR_SV[j-1];
                        PowerL_SV[j] = PowerL_SV[j-1];
                        Error_SV[j] = Error_SV[j-1];
                    }
                    PowerR_SV[1] = PowerR;
                    PowerL_SV[1] = PowerL;
                    Error_SV[1] = error;
                    rodo_prev = RODO;
                }
                error_prum = Error_SV[1]-Error_SV[20];
                power_prum = PowerR - PowerL;

                if((error_prum<100)&&(error_prum>-100)&&(RODO>10000)){
                    //cout << "error_prum: " << error_prum << "\n";

                    if((error_prum<1)&&(error_prum>-1)&&(power_prum>-100)&&(power_prum<100))
                    {
                        cout << "ACC" << "\n";
                        rodo_prev = RODO;
                        stav = 60;
                    }
                    /*if((error_prum>15)||(error_prum<-15))
                    {
                        cout << "SLOW" << "\n";
                        rodo_prev = RODO;
                        stav = 70;
                    }*/
                }


                if ((s0_hodnota>80)&&(s2_hodnota>80)&&(error < 10)){
                    cout << "CROSS" << "\n";
                    rodo_prev = RODO;
                    stav=20;
                }

                if ((s0_hodnota<20)&&(s2_hodnota<20)&&(error>20)){
                    cout << "SPACE" << "\n";
                    rodo_prev = RODO;
                    PowerR_Save = (PowerR_SV[20]+PowerR_SV[19]+PowerR_SV[18]+PowerR_SV[17]+PowerR_SV[16]+PowerR_SV[15]+PowerR_SV[14]+PowerR_SV[13]+PowerR_SV[12]+PowerR_SV[11])/10;
                    PowerL_Save = (PowerR_SV[20]+PowerL_SV[19]+PowerL_SV[18]+PowerL_SV[17]+PowerL_SV[16]+PowerL_SV[15]+PowerL_SV[14]+PowerL_SV[13]+PowerL_SV[12]+PowerL_SV[11])/10;

                    stav=50;
                }

                break;

                //CROSS
            case 20:

                SetSpeed(Right, offset);
                SetSpeed(Left, offset);

                if((RODO-rodo_prev)>2000){
                    cout << "DONE" << "\n";
                    lasterror=0;
                    error=0;
                    rodo_prev = RODO;
                    stav=10;
                }

                break;

                //SPACE
            case 50:
                if(s0_hodnota>70)
                {
                    PowerR_Save = PowerR_Save - 10;
                }
                if(s2_hodnota>70)
                {
                    PowerL_Save = PowerL_Save - 10;
                }

                SetSpeed(Left, PowerL_Save);
                SetSpeed(Right, PowerR_Save);
                if (((s0_hodnota>18)&&(s2_hodnota>18))&&(s1_hodnota>45)&&(s1_hodnota<55)){
                    lasterror=0;
                    error=0;
                    stav = 10;
                    rodo_prev = RODO;
                    cout << "DONE" << "\n";
                }
                break;

                //ACC
            case 60:
                //magic = 200;
                if((RODO-rodo_prev)>50){
                    offset_plus = offset_plus + 10;
                    //cout << "offset_plus: " << offset_plus << "\n";
                    rodo_prev=RODO;
                }

                if(offset_plus>1000)
                {
                    offset_plus = 1000;
                }

                SetSpeed(Right, (offset+offset_plus)-turn);
                SetSpeed(Left, (offset+offset_plus)+turn);

                if ((s0_hodnota>80)||(s2_hodnota>80)){
                    cout << "DONE" << "\n";
                    offset_plus = 0;
                    rodo_prev = RODO;
                    stav=10;
                }
                if ((s0_hodnota<20)&&(s2_hodnota<20)&&(error>20)){
                    cout << "SPACE" << "\n";
                    rodo_prev = RODO;
                    PowerR_Save = (PowerR_SV[20]+PowerR_SV[19]+PowerR_SV[18]+PowerR_SV[17]+PowerR_SV[16]+PowerR_SV[15]+PowerR_SV[14]+PowerR_SV[13]+PowerR_SV[12]+PowerR_SV[11])/10;
                    PowerL_Save = (PowerR_SV[20]+PowerL_SV[19]+PowerL_SV[18]+PowerL_SV[17]+PowerL_SV[16]+PowerL_SV[15]+PowerL_SV[14]+PowerL_SV[13]+PowerL_SV[12]+PowerL_SV[11])/10;

                    stav=50;
                }
                break;

            case 70:
                //magic = 200;
                if((RODO-rodo_prev)>10){
                    offset_plus = offset_plus + 100;
                    //cout << "offset_plus: " << offset_plus << "\n";
                    rodo_prev=RODO;
                }

                if(offset_plus>3000)
                {
                    offset_plus = 3000;
                }

                SetSpeed(Right, (offset-offset_plus)-turn);
                SetSpeed(Left, (offset-offset_plus)+turn);

                /*if ((s0_hodnota>80)||(s2_hodnota>80)){
                    cout << "DONE" << "\n";
                    magic = 70;
                    offset_plus = 0;
                    rodo_prev = RODO;
                    stav=10;
                }*/
                break;
        }
        lasterror = error;


        sleep_for(10ms);

    }}
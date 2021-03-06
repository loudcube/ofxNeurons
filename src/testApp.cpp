#include "testApp.h"

void testApp::setup(){

    ofSetWindowTitle("ofxNeurons");
    ofEnableAlphaBlending();
    frame_rate = 30;
    ofSetFrameRate(frame_rate);

    osc_enable = true;

    if (osc_enable){
        sender.setup("127.0.0.1", PORT_SEND);
        receiver.setup(PORT_RECEIVE);
    }

   // int mm = 20;
    TOTAL = 12;

    dc_mean = 4;
    dc_std = 0;
    syn_w_mean = 0;
    syn_w_std = 0;
    syn_d_mean = 0;
    syn_d_std = 0;
    global_dt = 0.01;

    Red.setup(TOTAL);
    Red.set_dts(global_dt);
    Red.set_currents(dc_mean,dc_std);

    Red.set_syn_w_matrix(0.5, syn_w_mean, syn_w_std);
    Red.set_syn_d_matrix(syn_d_mean, syn_d_std);

    if (osc_enable)
        Red.set_osc_server(&sender);

    sampleRate  = 44100;
    bufferSize = 512;

    verbose = true;

    soundStream.listDevices();
    soundStream.setDeviceID(0);
    soundStream.setup(this, 2, 0, sampleRate, bufferSize, 4);

}

void testApp::update(){

    if (osc_enable)
    {
        while(receiver.hasWaitingMessages()){
            // get the next message
            ofxOscMessage m;
            receiver.getNextMessage(&m);

            if(m.getAddress() == "/neurons_net/dc_mean_std" ){
                // both the arguments are int32's
                dc_mean = m.getArgAsFloat(0);
                dc_std = m.getArgAsFloat(1);
                Red.set_currents(dc_mean,dc_std);
            }

            if(m.getAddress() == "/neurons_net/all_neurons_tau" ){
                // both the arguments are int32's
                for(int i =0 ; i< TOTAL;i++)
                    Red.Neuronas[i].tau = MAX(m.getArgAsFloat(0),0.0001);
                if(verbose)
                    cout << "tau= " << m.getArgAsFloat(0) << endl;
            }

            if(m.getAddress() == "/neurons_net/syn_w_mean_std" ){

                type_prop = (float)m.getArgAsFloat(0);
                syn_w_mean = (float)m.getArgAsFloat(1);
                syn_w_std = (float)m.getArgAsFloat(2);
                Red.set_syn_w_matrix(type_prop, syn_w_mean,syn_w_std);
                if(verbose)
                    cout << "syn_w_mean= " << syn_w_mean << "\t" << "syn_w_std= " << syn_w_std << endl;
            }

            if(m.getAddress() == "/neurons_net/syn_d_mean_std" ){

                syn_d_mean = (float)m.getArgAsFloat(0);
                syn_d_std = (float)m.getArgAsFloat(1);
                Red.set_syn_d_matrix(syn_d_mean,syn_d_std);
                if(verbose)
                    cout << "syn_d_mean= " << syn_d_mean << "\t" << "syn_d_std= " << syn_d_std << endl;
            }

            if(m.getAddress() == "/neurons_net/neuron_dc" ){

                int n = (int)m.getArgAsFloat(0);
                n = (int) CLAMP(n,0,TOTAL-1);
                Red.Neuronas[n].dc =(float) m.getArgAsFloat(1);
                if(verbose)
                    cout << "neuron "<< n << " dc=" << Red.Neuronas[n].dc << endl;
            }

            if(m.getAddress() == "/neurons_net/neuron_type" ){

                int n = (int)m.getArgAsFloat(0);
                n = (int) CLAMP(n,0,TOTAL-1);
                Red.Neuronas[n].syn_type = (float) m.getArgAsFloat(1)*2-1;
                if(verbose)
                    cout << "neuron "<< n << " type=" << Red.Neuronas[n].dc << endl;
            }
            if(m.getAddress() == "/neurons_net/syn_w" ){

                int i =(int) m.getArgAsFloat(0);
                int j = (int) m.getArgAsFloat(1);
                i = (int) CLAMP(i,0,TOTAL-1);
                j = (int) CLAMP(j,0,TOTAL-1);
                Red.Matrix_Sinapsis[i][j].weigth =(float) m.getArgAsFloat(2);
                if(verbose)
                    cout << i<< " "<< j <<  " " << Red.Matrix_Sinapsis[i][j].weigth << endl;
            }

            if(m.getAddress() == "/neurons_net/syn_d" ){

                int i =(int) m.getArgAsFloat(0);
                int j = (int) m.getArgAsFloat(1);
                i = (int) CLAMP(i,0,TOTAL-1);
                j = (int) CLAMP(j,0,TOTAL-1);
                Red.Matrix_Sinapsis[i][j].delay =(float) m.getArgAsFloat(2);
                if(verbose)
                    cout << i<< " "<< j <<  " " << Red.Matrix_Sinapsis[i][j].delay << endl;
            }

            if(m.getAddress() == "/neurons_net/fr_set" ){

                float fc = m.getArgAsFloat(0);
                float Q = m.getArgAsFloat(1);
                for(int i =0 ; i< TOTAL;i++)
                    Red.Neuronas[i].FRset(fc,Q);
            }
        }
    }


    //Red.update();
}

void testApp::draw(){

   	//ofBackgroundGradient(ofColor::gray, ofColor::black, OF_GRADIENT_CIRCULAR);
   	ofSetWindowTitle(ofToString(ofGetFrameRate()));
   	ofBackground(ofColor::black);
    Red.draw();

}

//--------------------------------------------------------------
void testApp::audioOut(float * output, int bufferSize, int nChannels){

    for(int i=0; i<bufferSize; i++)
    {
        Red.update();

        float outL = 0,outR = 0;

        for(int j=0; j<TOTAL; j++)
        {
            outL +=  Red.Neuronas[j].Vnorm *(float)j/(float)TOTAL;
            outR +=  Red.Neuronas[j].Vnorm *(float)(TOTAL-j)/(float)TOTAL;
        }

        output[i*nChannels    ] = outL;
        output[i*nChannels + 1] = outR;
    }



}
//--------------------------------------------------------------
void testApp::keyPressed(int key){

}

//--------------------------------------------------------------
void testApp::keyReleased(int key){

    if(key=='1')
        Red.drawCircle = !Red.drawCircle;
    if(key=='2')
        Red.drawEvent = !Red.drawEvent;
    if(key=='3')
        Red.drawVoltage = !Red.drawVoltage;
    if(key=='4')
        Red.drawFR = !Red.drawFR;

    if(key=='r')
        Red.reset();

    if(key=='a')
    {
        dc_mean+= .05;
        cout << "dc_mean= " << dc_mean << "\t" << "dc_std= " << dc_std << endl;
        Red.set_currents(dc_mean,dc_std);
    }

    if(key=='z')
    {
        dc_mean-= .05;
        cout << "dc_mean= " << dc_mean << "\t" << "dc_std= " << dc_std << endl;
        Red.set_currents(dc_mean,dc_std);
    }

    if(key=='s')
    {
        dc_std+= 1;
        cout << "dc_mean= " <<dc_mean << "\t" << "dc_std= " << dc_std << endl;
        Red.set_currents(dc_mean,dc_std);
    }

    if(key=='x')
    {
        dc_std-= 1;
        cout << "dc_mean= " << dc_mean << "\t" << "dc_std= " << dc_std << endl;
        Red.set_currents(dc_mean,dc_std);
    }

    if(key=='o')
    {
        global_dt *= 1.1;
        cout << "global_dt= " << global_dt << endl;
        Red.set_dts(global_dt);
    }

    if(key=='o')
    {
        global_dt *= 1.1;
        cout << "global_dt= " << global_dt << endl;
        Red.set_dts(global_dt);
    }
    if(key=='l')
    {
        global_dt /= 1.1;
        cout << "global_dt= " << global_dt << endl;
        Red.set_dts(global_dt);
    }

    if(key=='u')
    {
        frame_rate++;
        ofSetFrameRate(frame_rate);
    }
    if(key=='j')
    {
        frame_rate--;
        ofSetFrameRate(frame_rate);
    }


    if(key=='w')
        {Red.togg_syn_matrix();
        cout << "Matrix is " << Red.bool_syn_matrix<< endl;}

    if(key=='q')
    {
        for(int i = 0; i < TOTAL; i++)
        {
           for(int j = 0; j < TOTAL; j++)
               cout << Red.Matrix_Sinapsis[i][j].weigth*Red.Neuronas[i].syn_type << "\t";
            cout << endl;
        }
    }

    if(key=='v')
        verbose = !verbose;

    if(key=='i')
    {
        cout << "V = ";
        for(int i = 0; i < TOTAL; i++)
            cout << Red.Neuronas[i].V << ",";

        cout << "\n";

        cout << "I = ";
        for(int i = 0; i < TOTAL; i++)
            cout << Red.Neuronas[i].I << ",";

        cout << "\n";

        cout << "FR = ";
        for(int i = 0; i < TOTAL; i++)
            cout << Red.Neuronas[i].FR<< ",";

        cout << "\n";
    }

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){

//
//        float auxX = (float)x/(float)ofGetWindowWidth();
//        float auxY = (float)y/(float)ofGetWindowHeight();
//
//        type_prop = auxX;
//        syn_w_mean = ofMap(1-auxY,0,1,0,80);
//        syn_w_std = 10;
//
//        //Red.set_syn_type();
//        Red.set_syn_matrix(type_prop, syn_w_mean, syn_w_std);
}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){

}

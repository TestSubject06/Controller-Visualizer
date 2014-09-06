#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>
#include <Xinput.h>
#include <iostream>
#include <map>
#include <fstream>
#include <sstream>
#include <wbemidl.h>
#include <oleauto.h>
#include <wbemidl.h>

#pragma comment(lib, "XInput.lib")
/*
Configurable controls. I'd like to make it so that the user can set what button press triggers each light.
Tap to set format? Run through each button one at a time and flicker it and wait for input?
*/

#define SAFE_RELEASE(p) { if ( (p) ) { (p)->Release(); (p) = 0; } }
#define SAFE_DELETE(a) if( (a) != NULL ) delete (a); (a) = NULL;

class Gamepad{
public:
	bool* buttons;
	float* axis;
	int index;
	int numButtons;
	int numAxes;
	bool _hasChanged;

	//Boolean used to check and see if the controller is still connected.
	virtual bool isConnected()=0;
	
	//Used to initialize the controller
	virtual void init(int index)=0;

	//Checks if the state of the gamepad has changed. In this case - we're only looking at button presses.
	virtual bool hasChanged()=0;

	//Updates the controllers
	virtual void update()=0;

	//Checks if a particular button was just pressed
	virtual bool justPressed(int buttonID)=0;

	//Checks if a particular button was just released
	virtual bool justReleased(int buttonID)=0;

	virtual ~Gamepad(){};

protected:
	//Past buttons are used for checking justPressed and justReleased states
	bool* pastButtons;
	float* pastAxis;
};

class SFMLGamepad : public Gamepad{
protected:
	sf::Joystick::Axis* axisBinding;
	sf::Joystick::Axis getAxis(int num){
		switch(num){
			case 0:
				return sf::Joystick::X;
			case 1:
				return sf::Joystick::Y;
			case 2:
				return sf::Joystick::Z;
			case 3:
				return sf::Joystick::R;
			case 4:
				return sf::Joystick::U;
			case 5:
				return sf::Joystick::V;
			case 6:
				return sf::Joystick::PovX;
			case 7:
				return sf::Joystick::PovY;
		}
		return sf::Joystick::X;
	}
public:
	void init(int index){
		this->index = index;
		//initialize the arrays
		numButtons = (int)sf::Joystick::getButtonCount(index);
		numAxes = 0;
		for(int i = 0; i < sf::Joystick::AxisCount; i++){
			if(sf::Joystick::hasAxis(index, getAxis(i))){
				numAxes++;
			}
		}
		axisBinding = (sf::Joystick::Axis*)malloc(sizeof(sf::Joystick::Axis)*numAxes);
		int bind = 0;
		for(int i = 0; i < sf::Joystick::AxisCount; i++){
			if(sf::Joystick::hasAxis(index, getAxis(i))){
				axisBinding[bind] = getAxis(i);
			}
		}
		buttons = (bool*)malloc(sizeof(bool)*numButtons);
		pastButtons = (bool*)malloc(sizeof(bool)*numButtons);
		axis = (float*)malloc(sizeof(float)*numAxes);
		pastAxis = (float*)malloc(sizeof(float)*numAxes);

		for(int i = 0; i < numButtons; i++){
			buttons[i] = sf::Joystick::isButtonPressed(index, i);
			pastButtons[i] = buttons[i];
		}
		for(int i = 0; i < numAxes; i++){
			axis[i] = sf::Joystick::getAxisPosition(index, axisBinding[i])/100.0f;
			pastAxis[i] = axis[i];
		}
	}

	~SFMLGamepad(){
		free(buttons);
		free(pastButtons);
		free(axis);
		free(pastAxis);
		free(axisBinding);
	}

	void update(){
		_hasChanged = false;
		for(int i = 0; i < numButtons; i++){
			pastButtons[i] = buttons[i];
			buttons[i] = sf::Joystick::isButtonPressed(index, i);
			if(pastButtons[i] != buttons[i]){
				_hasChanged = true;
			}
		}
		for(int i = 0; i < numAxes; i++){
			pastAxis[i] = axis[i];
			axis[i] = sf::Joystick::getAxisPosition(index, axisBinding[i])/100.0f;
		}
	}

	bool hasChanged(){
		return _hasChanged;
	}

	bool isConnected(){
		return sf::Joystick::isConnected(index);
	}

	bool justPressed(int buttonID){
		return (buttons[buttonID] && !pastButtons[buttonID]);
	}

	bool justReleased(int buttonID){
		return (buttons[buttonID] && !pastButtons[buttonID]);
	}
};

class XInputGamepad: public Gamepad{
protected:
	XINPUT_STATE controllerState;
	XINPUT_STATE getState(){
		XINPUT_STATE _controllerState;
		// Zeroise the state
		ZeroMemory(&_controllerState, sizeof(XINPUT_STATE));

		// Get the state
		XInputGetState(index, &_controllerState);

		return _controllerState;
	}
	bool isButtonDown(int buttonID){
		switch(buttonID){
		case 0:
			return (controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_A) != 0;
			break;

		case 1:
			return (controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_B) != 0;
			break;

		case 2:
			return (controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_X) != 0;
			break;

		case 3:
			return (controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_Y) != 0;
			break;

		case 4:
			return (controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) != 0;
			break;

		case 5:
			return (controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) != 0;
			break;

		case 6:
			return (controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB) != 0;
			break;

		case 7:
			return (controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB) != 0;
			break;

		case 8:
			return (controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_BACK) != 0;
			break;

		case 9:
			return (controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_START) != 0;
			break;

		case 10:
			return (controllerState.Gamepad.bLeftTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD);
			break;

		case 11:
			return (controllerState.Gamepad.bRightTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD);
			break;
		}
		return false;
	}

	float getAxis(int axisID){
		switch(axisID){
		case 0:
			return controllerState.Gamepad.sThumbLX / 32767.5f;
			break;

		case 1:
			return controllerState.Gamepad.sThumbLY / 32767.5f;
			break;

		case 2:
			return controllerState.Gamepad.sThumbRX / 32767.5f;
			break;

		case 3:
			return controllerState.Gamepad.sThumbRY / 32767.5f;
			break;

		case 4:{
			int axisValue = 0;
			axisValue += (controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT) ? -1 : 0;
			axisValue += (controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) ? 1 : 0;
			return (float)axisValue;
			break;
			}

		case 5:{
			int axisValuer = 0;
			axisValuer += (controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP) ? -1 : 0;
			axisValuer += (controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) ? 1 : 0;
			return (float)axisValuer;
			break;
			}
		}
		return 0.0f;
	}
public:
	XInputGamepad(int i) {
		index = i;
	}
	void init(int index){
		this->index = index;
		//initialize the arrays
		numButtons = 12;
		numAxes = 6;
		
		buttons = (bool*)malloc(sizeof(bool)*numButtons);
		pastButtons = (bool*)malloc(sizeof(bool)*numButtons);
		axis = (float*)malloc(sizeof(float)*numAxes);
		pastAxis = (float*)malloc(sizeof(float)*numAxes);

		controllerState = getState();

		for(int i = 0; i < numButtons; i++){
			buttons[i] = isButtonDown(i);
			pastButtons[i] = buttons[i];
		}
		
		for(int i = 0; i < numAxes; i++){
			axis[i] = getAxis(i);
			pastAxis[i] = axis[i];
		}
	}

	~XInputGamepad(){
		free(buttons);
		free(pastButtons);
		free(axis);
		free(pastAxis);
	}

	void update(){
		controllerState = getState();
		_hasChanged = false;
		for(int i = 0; i < numButtons; i++){
			pastButtons[i] = buttons[i];
			buttons[i] = isButtonDown(i);
			if(pastButtons[i] != buttons[i]){
				_hasChanged = true;
			}
		}
		for(int i = 0; i < numAxes; i++){
			pastAxis[i] = axis[i];
			axis[i] = getAxis(i);
		}
	}

	bool hasChanged(){
		return _hasChanged;
	}

	bool isConnected(){
		XINPUT_STATE state;
		ZeroMemory(&state, sizeof(XINPUT_STATE));
		DWORD result = XInputGetState(index, &state);
		if(result == ERROR_SUCCESS){
			return true;
		}
		return false;
	}

	bool justPressed(int buttonID){
		return (buttons[buttonID] && !pastButtons[buttonID]);
	}

	bool justReleased(int buttonID){
		return (buttons[buttonID] && !pastButtons[buttonID]);
	}
};

struct Thumbstick{
	int behavior;
	int distance;
	sf::Vector2f position;
	sf::Vector2f anchor;
	int x_axis;
	int y_axis;
	int z_axis;
	float maxTheta;
	Thumbstick()
		:behavior(0),
		 x_axis(INT_MAX),
		 y_axis(INT_MAX),
		 z_axis(INT_MAX),
		 maxTheta(35.0f),
		 distance(20),
		 position(0.0f, 0.0f),
		 anchor(0.0f, 0.0f)
	{}
};

//Part of the refactor
sf::Sprite** controllerSprites;
sf::Texture** controllerImages;
sf::Vector2f axes[8];
Thumbstick** thumbsticks;
int numThumbsticks = 0;

sf::Vector2f thumbPos1;
sf::Vector2f thumbPos2;
std::vector<std::string> keys, values, lines;
std::string state, directory;
std::map<std::string, std::string> configFileMap;
//bool buttons[32];
bool noGamepad = false;
bool ready = false;
bool waiting = false;
bool claimed[4] = {false, false, false, false};
int ACTIVE_GAMEPAD = -1;
int numGamepads = 0;
Gamepad** GAMEPADS;
std::stringstream ss;

//Stuff for tap to config
bool config = false;
bool flicker = false;
int buttonConfig = 0;
int* rebinds;


bool IsXInputDevice(unsigned int pid, unsigned int vid)
{
    IWbemLocator*           pIWbemLocator  = NULL;
    IEnumWbemClassObject*   pEnumDevices   = NULL;
    IWbemClassObject*       pDevices[20]   = {0};
    IWbemServices*          pIWbemServices = NULL;
    BSTR                    bstrNamespace  = NULL;
    BSTR                    bstrDeviceID   = NULL;
    BSTR                    bstrClassName  = NULL;
    DWORD                   uReturned      = 0;
    bool                    bIsXinputDevice= false;
    UINT                    iDevice        = 0;
    VARIANT                 var;
    HRESULT                 hr;

    // CoInit if needed
    hr = CoInitialize(NULL);
    bool bCleanupCOM = SUCCEEDED(hr);

    // Create WMI
    hr = CoCreateInstance( __uuidof(WbemLocator),
                           NULL,
                           CLSCTX_INPROC_SERVER,
                           __uuidof(IWbemLocator),
                           (LPVOID*) &pIWbemLocator);
    if( FAILED(hr) || pIWbemLocator == NULL )
        goto LCleanup;

    bstrNamespace = SysAllocString( L"\\\\.\\root\\cimv2" );if( bstrNamespace == NULL ) goto LCleanup;        
    bstrClassName = SysAllocString( L"Win32_PNPEntity" );   if( bstrClassName == NULL ) goto LCleanup;        
    bstrDeviceID  = SysAllocString( L"DeviceID" );          if( bstrDeviceID == NULL )  goto LCleanup;        
    
    // Connect to WMI 
    hr = pIWbemLocator->ConnectServer( bstrNamespace, NULL, NULL, 0L, 
                                       0L, NULL, NULL, &pIWbemServices );
    if( FAILED(hr) || pIWbemServices == NULL )
        goto LCleanup;

    // Switch security level to IMPERSONATE. 
    CoSetProxyBlanket( pIWbemServices, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL, 
                       RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE );                    

    hr = pIWbemServices->CreateInstanceEnum( bstrClassName, 0, NULL, &pEnumDevices ); 
    if( FAILED(hr) || pEnumDevices == NULL )
        goto LCleanup;

    // Loop over all devices
    for( ;; )
    {
        // Get 20 at a time
        hr = pEnumDevices->Next( 10000, 20, pDevices, &uReturned );
        if( FAILED(hr) )
            goto LCleanup;
        if( uReturned == 0 )
            break;

        for( iDevice=0; iDevice<uReturned; iDevice++ )
        {
            // For each device, get its device ID
            hr = pDevices[iDevice]->Get( bstrDeviceID, 0L, &var, NULL, NULL );
            if( SUCCEEDED( hr ) && var.vt == VT_BSTR && var.bstrVal != NULL )
            {
                // Check if the device ID contains "IG_".  If it does, then it's an XInput device
				    // This information can not be found from DirectInput 
                if( wcsstr( var.bstrVal, L"IG_" ) )
                {
                    // If it does, then get the VID/PID from var.bstrVal
                    DWORD dwPid = 0, dwVid = 0;
                    WCHAR* strVid = wcsstr( var.bstrVal, L"VID_" );
                    if( strVid && swscanf_s( strVid, L"VID_%4X", &dwVid ) != 1 )
                        dwVid = 0;
                    WCHAR* strPid = wcsstr( var.bstrVal, L"PID_" );
                    if( strPid && swscanf_s( strPid, L"PID_%4X", &dwPid ) != 1 )
                        dwPid = 0;
					//ss << " XInput results: " << dwPid << " " << dwVid;
					if(dwPid == pid && dwVid == vid){
						bIsXinputDevice = true;
						goto LCleanup;
					}
                }
            }   
            SAFE_RELEASE( pDevices[iDevice] );
        }
    }

LCleanup:
    if(bstrNamespace)
        SysFreeString(bstrNamespace);
    if(bstrDeviceID)
        SysFreeString(bstrDeviceID);
    if(bstrClassName)
        SysFreeString(bstrClassName);
    for( iDevice=0; iDevice<20; iDevice++ )
        SAFE_RELEASE( pDevices[iDevice] );
    SAFE_RELEASE( pEnumDevices );
    SAFE_RELEASE( pIWbemLocator );
    SAFE_RELEASE( pIWbemServices );

    if( bCleanupCOM )
        CoUninitialize();

    return bIsXinputDevice;
}

void checkControllers(sf::Text*){
	//Find the possible contollers;
	if(GAMEPADS != NULL){
		for(int i = 0; i < 8; i++){
			delete GAMEPADS[i];
			GAMEPADS[i] = NULL;
		}
	}
	delete GAMEPADS;
	GAMEPADS = NULL;
	GAMEPADS = new Gamepad*[8];
	for(int i = 0; i < 8; i++){
		GAMEPADS[i] = NULL;
		if(sf::Joystick::isConnected(i)){
			//ss << " DirectInput? results: " << sf::Joystick::getIdentification(i).productId << " " << sf::Joystick::getIdentification(i).vendorId;
			if(IsXInputDevice(sf::Joystick::getIdentification(i).productId, sf::Joystick::getIdentification(i).vendorId)){
				int initializer = 0;
				GAMEPADS[numGamepads] = new XInputGamepad(initializer);
				while(!claimed[GAMEPADS[numGamepads]->index]){
					GAMEPADS[numGamepads]->init(initializer++);
					if(GAMEPADS[numGamepads]->isConnected()){
						claimed[GAMEPADS[numGamepads]->index] = true;
						numGamepads++;
						break;
					}else{
						GAMEPADS[numGamepads]->~Gamepad();
					}
				}
			}else{
				GAMEPADS[numGamepads] = new SFMLGamepad();
				GAMEPADS[numGamepads]->init(i);
				numGamepads++;
			}
		}
	}
	if(numGamepads == 1){
		ACTIVE_GAMEPAD = GAMEPADS[0]->index;
		noGamepad = false;
		ready = true;
	}else if(numGamepads == 0){
		//No gamepads.
		ss << "There are no compatable gamepads connected." << "\n" 
			<< "Press F1 to refresh";
		noGamepad = true;
		ready = false;
	}else{
		ss << "There are multiple gamepads connected." << "\n" 
			<< "Press a button on the one you want" << "\n"
			<< "to be displayed.";
		waiting = true;
		noGamepad = false;
	}
	return;
}

int stringToInt(std::string s){
	return atoi(s.c_str());
}

/*
void allocateSpriteMemory(){
	if(controllerImages != NULL){
		free(controllerImages);
		controllerImages = NULL;
		free(controllerSprites);
		controllerSprites = NULL;
	}
	int numImages = stringToInt(configFileMap["numImages"]);
	
	controllerImages = (sf::Texture*)malloc(sizeof(sf::Texture)*numImages);
	controllerSprites = (sf::Sprite*)malloc(sizeof(sf::Sprite)*numImages);
}*/

int main()
{
	state = "directory";
	std::string line;
	std::ifstream configFile("config.txt");
	if(configFile.is_open()){
		std::string line;
		while(configFile.good()){
			std::getline(configFile, line);
			lines.push_back(line);

			std::istringstream is_line(line);
			std::string key;
			if( std::getline(is_line, key, '=') )
			{
				std::string value;
				if(std::getline(is_line, value)){
					configFileMap[key] = value;
				}
			}
		}
	}

	//Load in images abstractly
	//allocateSpriteMemory();
	sf::Texture t;
	int numImages = stringToInt(configFileMap["numImages"]);
	int tcount = 0;
	int bcount = 0;
	controllerImages = new sf::Texture*[numImages];
	controllerSprites = new sf::Sprite*[numImages];
	std::stringstream helper;
	for(int i = 0; i < numImages; i++){
		controllerImages[i] = new sf::Texture();
		std::string key;
		if(i==0){
			key = "base";
			if(!controllerImages[i]->loadFromFile(configFileMap["directory"] + configFileMap[key])){
				ss<<"uh oh\n";
			}
		}else{
			helper<<"thumbstick"<<(tcount+1);
			if(configFileMap.count(helper.str())){
				tcount++;
				key = helper.str();
				controllerImages[i]->loadFromFile(configFileMap["directory"] + configFileMap[key]);
				helper.str("");
			}else{
				helper.str("");
				helper<<"button"<<(bcount);
				if(configFileMap.count(helper.str())){
					key = helper.str();
					controllerImages[i]->loadFromFile(configFileMap["directory"] + configFileMap[key]);
					bcount++;
				}
			}
		}

		controllerSprites[i] = new sf::Sprite(*controllerImages[i]);
		//setLocation of the sprites here.
		controllerSprites[i]->setPosition(stringToInt(configFileMap[key+"x"])+.35f, stringToInt(configFileMap[key+"y"])+.35f);
	}

	//Initialize the thumbsticks.
	//A thumbstick is anything that is moveable on the controller image, really.
	numThumbsticks = tcount;
	thumbsticks = new Thumbstick*[numThumbsticks];
	std::stringstream ns;
	for(int i = 0; i < numThumbsticks; i++){
		ns << "thumbstick" << i+1;
		thumbsticks[i] = new Thumbstick();
		thumbsticks[i]->anchor.x = stringToInt(configFileMap[ns.str() + "x"])+.35f;
		thumbsticks[i]->anchor.y = stringToInt(configFileMap[ns.str() + "y"])+.35f;
		thumbsticks[i]->distance = stringToInt(configFileMap[ns.str() + "distance"]);
		if(configFileMap.count(ns.str() + "movementtype") != 0){
			thumbsticks[i]->behavior = configFileMap[ns.str() + "movementtype"].compare("circle");
		}
		if(configFileMap.count(ns.str() + "xaxis") != 0){
			thumbsticks[i]->x_axis = stringToInt(configFileMap[ns.str() + "xaxis"]);
		}
		if(configFileMap.count(ns.str() + "yaxis") != 0){
			thumbsticks[i]->y_axis = stringToInt(configFileMap[ns.str() + "yaxis"]);
		}
		if(configFileMap.count(ns.str() + "zaxis") != 0){
			thumbsticks[i]->z_axis = stringToInt(configFileMap[ns.str() + "zaxis"]);
		}
		if(configFileMap.count(ns.str() + "maxrotation") != 0){
			thumbsticks[i]->maxTheta = (float)stringToInt(configFileMap[ns.str() + "maxrotation"]);
		}
		ns.str("");
	}
	ss<<thumbsticks[0]->behavior<<"\n";
	ss<<thumbsticks[1]->behavior<<"\n";
	//Load in images.
	//PS3 Controller texture
	/*
	sf::Texture tex;
	if (!tex.loadFromFile(configFileMap["directory"] + configFileMap["base"]))
	{
		// Error...
	}
	sf::Sprite ps(tex);

	//Thumbstick1 texture
	sf::Texture texThumb1;
	if (!texThumb1.loadFromFile(configFileMap["directory"] + configFileMap["thumbstick1"]))
	{
		// Error...
	}

	//Thumbstick2 texture
	sf::Texture texThumb2;
	if (!texThumb2.loadFromFile(configFileMap["directory"] + configFileMap["thumbstick2"]))
	{
		// Error...
	}

	//Cross
	sf::Texture texCross;
	if (!texCross.loadFromFile(configFileMap["directory"] + configFileMap["button2"]))
	{
		// Error...
	}

	//Triangle
	sf::Texture texTriangle;
	if (!texTriangle.loadFromFile(configFileMap["directory"] + configFileMap["button0"]))
	{
		// Error...
	}

	//Circle
	sf::Texture texCircle;
	if (!texCircle.loadFromFile(configFileMap["directory"] + configFileMap["button1"]))
	{
		// Error...
	}

	//Square
	sf::Texture texSquare;
	if (!texSquare.loadFromFile(configFileMap["directory"] + configFileMap["button3"]))
	{
		// Error...
	}

	//Start
	sf::Texture texStart;
	if (!texStart.loadFromFile(configFileMap["directory"] + configFileMap["button11"]))
	{
		// Error...
	}

	//Select
	sf::Texture texSelect;
	if (!texSelect.loadFromFile(configFileMap["directory"] + configFileMap["button8"]))
	{
		// Error...
	}

	//R1
	sf::Texture texR1;
	if (!texR1.loadFromFile(configFileMap["directory"] + configFileMap["button5"]))
	{
		// Error...
	}

	//L
	sf::Texture texL1;
	if (!texL1.loadFromFile(configFileMap["directory"] + configFileMap["button4"]))
	{
		// Error...
	}

	//R2
	sf::Texture texR2;
	if (!texR2.loadFromFile(configFileMap["directory"] + configFileMap["button7"]))
	{
		// Error...
	}

	//L2
	sf::Texture texL2;
	if (!texL2.loadFromFile(configFileMap["directory"] + configFileMap["button6"]))
	{
		// Error...
	}

	//DirectionUP
	sf::Texture texDirectionUP;
	if (!texDirectionUP.loadFromFile(configFileMap["directory"] + configFileMap["button15"]))
	{
		// Error...
	}

	//DirectionDOWN
	sf::Texture texDirectionDOWN;
	if (!texDirectionDOWN.loadFromFile(configFileMap["directory"] + configFileMap["button16"]))
	{
		// Error...
	}

	//DirectionLEFT
	sf::Texture texDirectionLEFT;
	if (!texDirectionLEFT.loadFromFile(configFileMap["directory"] + configFileMap["button13"]))
	{
		// Error...
	}

	//DirectionRIGHT
	sf::Texture texDirectionRIGHT;
	if (!texDirectionRIGHT.loadFromFile(configFileMap["directory"] + configFileMap["button14"]))
	{
		// Error...
	}

	//PSButton
	sf::Texture texPSButton;
	if (!texPSButton.loadFromFile(configFileMap["directory"] + configFileMap["button12"]))
	{
		// Error...
	}

	*/

	// Create the main window
	sf::RenderWindow App;
	App.create(sf::VideoMode::VideoMode(controllerImages[0]->getSize().x + 800, controllerImages[0]->getSize().y+100), "PS3 Input Visualwhocares no one can read this", sf::Style::Default);
	App.setVerticalSyncEnabled(true);

	/*
	sf::Sprite thumb(texThumb1);
	thumb.setPosition(112.35f, 212.35f);

	sf::Sprite thumb2(texThumb2);
	thumb2.setPosition(234.35f, 212.35f);

	sf::Sprite Triangle(texTriangle);
	Triangle.setPosition(303.35f, 138.35f);

	sf::Sprite Circle(texCircle);
	Circle.setPosition(337.35f, 168.35f);

	sf::Sprite Square(texSquare);
	Square.setPosition(270.35f, 168.35f);

	sf::Sprite Cross(texCross);
	Cross.setPosition(304.35f, 198.35f);

	sf::Sprite DirectionLeft(texDirectionLEFT);
	DirectionLeft.setPosition(48.35f, 171.35f);

	sf::Sprite DirectionRight(texDirectionRIGHT);
	DirectionRight.setPosition(88.35f, 171.35f);

	sf::Sprite DirectionUp(texDirectionUP);
	DirectionUp.setPosition(69.35f, 150.35f);

	sf::Sprite DirectionDown(texDirectionDOWN);
	DirectionDown.setPosition(68.35f, 190.35f);

	sf::Sprite R1(texR1);
	R1.setPosition(291.35f, 65.35f);

	sf::Sprite L1(texL1);
	L1.setPosition(55.35f, 64.35f);

	sf::Sprite R2(texR2);
	R2.setPosition(292.35f, 3.35f);

	sf::Sprite L2(texL2);
	L2.setPosition(58.35f, 3.35f);

	sf::Sprite Start(texStart);
	Start.setPosition(225.35f, 176.35f);

	sf::Sprite Select(texSelect);
	Select.setPosition(153.35f, 177.35f);

	sf::Sprite PSButton(texPSButton);
	PSButton.setPosition(186.35f, 193.35f);
	*/

	thumbPos1.x = 200.f;
	thumbPos1.y = 200.f;

	
	sf::Font fnt;
	if(!fnt.loadFromFile("arial.ttf")){
		//error
	}
	sf::Text text("blah\nblah", fnt, 18);
	text.setColor(sf::Color::White);

	checkControllers(&text);
	rebinds = new int[20];
	for(int i = 0; i < 20; i++) rebinds[i] = i;
    // Start main loop
    bool Running = true;
	sf::Event Event;
    while (Running)
    {
		while(App.pollEvent(Event)){
			if(Event.type == Event.Closed || (Event.type == sf::Event::KeyPressed && Event.key.code == sf::Keyboard::Escape)){
				Running = false;
				App.close();
			}
			if(Event.type == sf::Event::KeyPressed && Event.key.code == sf::Keyboard::F1){
				ready = false;
				waiting = false;
				noGamepad = false;
				ACTIVE_GAMEPAD = -1;
				numGamepads = 0;
				checkControllers(&text);
			}
			if (Event.type == sf::Event::KeyPressed && Event.key.code == sf::Keyboard::F2){
				config = true;
				waiting = false;
				ready = false;
				//sf::Image image;
				//image = App.capture();
				//image.saveToFile("screenshot.jpg");
			}
		}
		if(waiting){
			for(int i = 0; i < numGamepads; i++){
				GAMEPADS[i]->update();
				if(GAMEPADS[i]->hasChanged()){
					ACTIVE_GAMEPAD = GAMEPADS[i]->index;
					ss << ACTIVE_GAMEPAD;
					ready = true;
					waiting = false;
				}
			}
		}
		if(ready){
			//for(int i = 0; i < GAMEPADS[ACTIVE_GAMEPAD]->numButtons; i++){
				//buttons[i] = false;
			//}

			for(int i = 0; i < numThumbsticks; i++){
				thumbsticks[i]->position.x = (thumbsticks[i]->x_axis < 0 ? -1 : 1)*GAMEPADS[ACTIVE_GAMEPAD]->axis[abs(thumbsticks[i]->x_axis)];
				thumbsticks[i]->position.y = (thumbsticks[i]->y_axis < 0 ? -1 : 1)*GAMEPADS[ACTIVE_GAMEPAD]->axis[abs(thumbsticks[i]->y_axis)];
				if(thumbsticks[i]->behavior == 0){
					float l = sqrt(thumbsticks[i]->position.x*thumbsticks[i]->position.x + thumbsticks[i]->position.y*thumbsticks[i]->position.y);
					if(l > 1){
						thumbsticks[i]->position.x = (thumbsticks[i]->position.x / l) * thumbsticks[i]->distance + (thumbsticks[i]->anchor.x+.35f);
						thumbsticks[i]->position.y = (thumbsticks[i]->position.y / l) * thumbsticks[i]->distance + (thumbsticks[i]->anchor.y+.35f);
					}else{
						thumbsticks[i]->position.x = thumbsticks[i]->position.x * thumbsticks[i]->distance + (thumbsticks[i]->anchor.x+.35f);
						thumbsticks[i]->position.y = thumbsticks[i]->position.y * thumbsticks[i]->distance + (thumbsticks[i]->anchor.y+.35f);
					}
				}else{
					thumbsticks[i]->position.x = thumbsticks[i]->position.x * thumbsticks[i]->distance + (thumbsticks[i]->anchor.x+.35f);
					thumbsticks[i]->position.y = thumbsticks[i]->position.y * thumbsticks[i]->distance + (thumbsticks[i]->anchor.y+.35f);
				}
			}

			/*
			thumbPos1.x = GAMEPADS[ACTIVE_GAMEPAD]->axis[0];
			thumbPos1.y = GAMEPADS[ACTIVE_GAMEPAD]->axis[1];
			float l = sqrt(thumbPos1.x*thumbPos1.x + thumbPos1.y*thumbPos1.y);
			if(l > 1){
				thumbPos1.x = (thumbPos1.x / l) * 20 + 112.35f;
				thumbPos1.y = (thumbPos1.y / l) * 20 + 212.35f;
			}else{
				thumbPos1.x = thumbPos1.x * 20 + 112.35f;
				thumbPos1.y = thumbPos1.y * 20 + 212.35f;
			}


			thumbPos2.x = GAMEPADS[ACTIVE_GAMEPAD]->axis[2];
			thumbPos2.y = GAMEPADS[ACTIVE_GAMEPAD]->axis[3];
			l = sqrt(thumbPos2.x*thumbPos2.x + thumbPos2.y*thumbPos2.y);
			if(l > 1){
				thumbPos2.x = (thumbPos2.x / l) * 20 + 234.35f;
				thumbPos2.y = (thumbPos2.y / l) * 20 + 212.35f;
			}else{
				thumbPos2.x = thumbPos2.x * 20 + 234.35f;
				thumbPos2.y = thumbPos2.y * 20 + 212.35f;
			}
			*/

			

			/*
			if(sf::Joystick::isButtonPressed(ACTIVE_GAMEPAD, 0)){
				buttons[0] = true;
			}
			if(sf::Joystick::isButtonPressed(ACTIVE_GAMEPAD, 1)){
				buttons[1] = true;
			}
			if(sf::Joystick::isButtonPressed(ACTIVE_GAMEPAD, 2)){
				buttons[2] = true;
			}
			if(sf::Joystick::isButtonPressed(ACTIVE_GAMEPAD, 3)){
				buttons[3] = true;
			}
			if(sf::Joystick::isButtonPressed(ACTIVE_GAMEPAD, 4)){
				buttons[4] = true;
			}
			if(sf::Joystick::isButtonPressed(ACTIVE_GAMEPAD, 5)){
				buttons[5] = true;
			}
			if(sf::Joystick::isButtonPressed(ACTIVE_GAMEPAD, 6)){
				buttons[6] = true;
			}
			if(sf::Joystick::isButtonPressed(ACTIVE_GAMEPAD, 7)){
				buttons[7] = true;
			}
			if(sf::Joystick::isButtonPressed(ACTIVE_GAMEPAD, 8)){
				buttons[8] = true;
			}
			if(sf::Joystick::isButtonPressed(ACTIVE_GAMEPAD, 11)){
				buttons[9] = true;
			}
			if(sf::Joystick::isButtonPressed(ACTIVE_GAMEPAD, 12)){
				buttons[10] = true;
			}
			*/

			//Dpad madness
			//No seriously. What the fuck are these goddamned values.
			if(sf::Joystick::getAxisPosition(ACTIVE_GAMEPAD, sf::Joystick::PovX) < -5){
				//buttons[11] = true;
			}else if(sf::Joystick::getAxisPosition(ACTIVE_GAMEPAD, sf::Joystick::PovX) < 100 && sf::Joystick::getAxisPosition(ACTIVE_GAMEPAD, sf::Joystick::PovX) > 5){
				//buttons[12] = true;
			}
			if(sf::Joystick::getAxisPosition(ACTIVE_GAMEPAD, sf::Joystick::PovY) < 50 && sf::Joystick::getAxisPosition(ACTIVE_GAMEPAD, sf::Joystick::PovY) != 0){
				//buttons[13] = true;
			}else if(sf::Joystick::getAxisPosition(ACTIVE_GAMEPAD, sf::Joystick::PovY) > 80){
				//buttons[14] = true;
			}else if(sf::Joystick::getAxisPosition(ACTIVE_GAMEPAD, sf::Joystick::PovY) == 0 && sf::Joystick::getAxisPosition(ACTIVE_GAMEPAD, sf::Joystick::PovX) == 100){ //SPECIAL CASE-O-RAMA
				//buttons[13] = true;
			}
		}

		// Set the active window before using OpenGL commands
        // It's useless here because active window is always the same,
        // but don't forget it if you use multiple windows or controls
        App.setActive();
		App.clear();
		App.pushGLStates();

		if(config){
			App.draw(*controllerSprites[0]);
			if(flicker){
				App.draw(*controllerSprites[buttonConfig + numThumbsticks + 1]);
			}
			flicker = !flicker;

			GAMEPADS[ACTIVE_GAMEPAD]->update();
			if(GAMEPADS[ACTIVE_GAMEPAD]->hasChanged()){
				for(int i = 0; i < GAMEPADS[ACTIVE_GAMEPAD]->numButtons; i++){
					if(GAMEPADS[ACTIVE_GAMEPAD]->buttons[i]){
						rebinds[buttonConfig] = i;
						buttonConfig++;
						if(buttonConfig > GAMEPADS[ACTIVE_GAMEPAD]->numButtons){
							config = false;
							ready = true;
						}
					}
				}
			}
		}

		if(ready){
			GAMEPADS[ACTIVE_GAMEPAD]->update();
			App.draw(*controllerSprites[0]);
			for(int i = 0; i < numThumbsticks; i++){
				controllerSprites[i+1]->setPosition(thumbsticks[i]->position);
				App.draw(*controllerSprites[i+1]);
			}
			
			for(int i = 0; i < GAMEPADS[ACTIVE_GAMEPAD]->numButtons; i++){
				if(GAMEPADS[ACTIVE_GAMEPAD]->buttons[rebinds[i]]){
					App.draw(*controllerSprites[i+(numThumbsticks+1)]);
				}
			}
			/*
			if(buttons[0]){
				App.draw(Triangle);
			}
			if(buttons[1]){
				App.draw(Circle);
			}
			if(buttons[2]){
				App.draw(Cross);
			}
			if(buttons[3]){
				App.draw(Square);
			}
			if(buttons[4]){
				App.draw(L1);
			}
			if(buttons[5]){
				App.draw(R1);
			}
			if(buttons[6]){
				App.draw(L2);
			}
			if(buttons[7]){
				App.draw(R2);
			}
			if(buttons[8]){
				App.draw(Select);
			}
			if(buttons[9]){
				App.draw(Start);
			}
			if(buttons[10]){
				App.draw(PSButton);
			}
			if(buttons[11]){
				App.draw(DirectionLeft);
			}
			if(buttons[12]){
				App.draw(DirectionRight);
			}
			if(buttons[13]){
				App.draw(DirectionUp);
			}
			if(buttons[14]){
				App.draw(DirectionDown);
			}
			*/
		}else if(!ready){
			//App.draw(text);
		}

		//ss << lines.size() << " " << configFileMap["base"];
		//text.setString("" + ss.str());
		//ss.str("");
		//ss.clear();
		text.setString(ss.str());
		//ss.str("");
		//ss.clear();

		App.draw(text);
		App.popGLStates();
        App.display();
    }
	
    return EXIT_SUCCESS;
}

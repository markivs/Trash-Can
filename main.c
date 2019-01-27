#pragma config(Sensor, S2,     infrared,       sensorEV3_IRSensor, modeEV3IR_Remote)
#pragma config(Sensor, S3,     ultrasonic,     sensorEV3_Ultrasonic)
#pragma config(Sensor, S4,     color,          sensorEV3_Color, modeEV3Color_Reflected_Raw)
#pragma config(Motor,  motorA,          usMotor,       tmotorEV3_Large, PIDControl, encoder)
#pragma config(Motor,  motorB,          leftMotor,     tmotorEV3_Large, PIDControl, encoder)
#pragma config(Motor,  motorC,          rightMotor,    tmotorEV3_Large, PIDControl, encoder)
#pragma config(Motor,  motorD,          irMotor,       tmotorEV3_Large, PIDControl, encoder)

// scan frequency in degrees
int vehicleSpeed = 10;
char state = 's';
float turnRatio = 0;
const int usScanFreq = 5;
const int killCode = 6;
const int followingDistance = 50;
const int scanFreq = 10;
// scan speed of motor
int motorScanSpeed = 10;
// array to store values of distances in each direction, depending on scan frequency
const int arrLength = 360/usScanFreq;
const int userLocArrLength = 360/scanFreq;
int dirArr[arrLength];
int dirArrIndex = arrLength/2;
int taskThreshold = 5;
bool irScanDir = false;
int userLocScan[userLocArrLength];

void turnRight(const int amotor, int degrees)
{
	moveMotorTarget(amotor, degrees, 10);
	waitUntilMotorStop(amotor);
}
void turnLeft(const int amotor, int degrees)
{
		moveMotorTarget(amotor, -degrees, -10);
		waitUntilMotorStop(amotor);
}
void turnVehicleRight(int degrees)
{
	moveMotorTarget(rightMotor, degrees*turnRatio, vehicleSpeed);
	moveMotorTarget(leftMotor, -degrees*turnRatio, -vehicleSpeed);
	waitUntilMotorStop(rightMotor);
}
void turnVehicleLeft(int degrees)
{
	moveMotorTarget(leftMotor, degrees*turnRatio, vehicleSpeed);
	moveMotorTarget(rightMotor, -degrees*turnRatio, -vehicleSpeed);
	waitUntilMotorStop(leftMotor);
}
void moveVehicle(int distance)
{
	int moveRatio = 10;
	moveMotorTarget(leftMotor, distance*moveRatio, vehicleSpeed);
	moveMotorTarget(rightMotor, distance*moveRatio, vehicleSpeed);
}
bool lightSensed()
{
	return (SensorValue(color) >= 10);
}
void USScan()
{
	if(dirArrIndex >= arrLength-1 || dirArrIndex <= 0)
	{
		irScanDir = !irScanDir;
	}
	if(irScanDir)
	{
		dirArrIndex++;
		turnRight(usMotor, usScanFreq);
	}
	else
	{
		dirArrIndex--;
		turnLeft(usMotor, usScanFreq);
	}n

	dirArr[dirArrIndex] = getUSDistance(ultrasonic);
}

void resetMotor(int amotor)
{
	while(getMotorEncoder(amotor) < -taskThreshold)
	{
		turnRight(amotor, 5);
	}
	while(getMotorEncoder(amotor) > taskThreshold)
	{
		turnLeft(amotor, 5);
	}
}
int trackIRDir(int threshold)
{
	int minIndex = 0;
	irScanDir = !irScanDir;
	if(irScanDir)
	{
		for(int i = 0; i < userLocArrLength; i++)
		{
			userLocScan[i] = getIRBeaconStrength(infrared);
			turnRight(irMotor, scanFreq);
			if(userLocScan[i] < userLocScan[minIndex])
			{
				minIndex = i;
			}
		}
	}
	if(!irScanDir)
	{
		for(int i = userLocArrLength-1; i >= 0; i--)
		{
			userLocScan[i] = getIRBeaconStrength(infrared);
			turnLeft(irMotor, scanFreq);
			if(userLocScan[i] < userLocScan[minIndex])
			{
				minIndex = i;
			}
		}
	}
	return minIndex*scanFreq;

}
char tracking()
{
	int threshold = 0;
	while(getIRBeaconDirection(infrared) != threshold)
	{
		int rotationAmount = 50;
		if(getMotorEncoder(irMotor) > 180)
		{
			turnLeft(irMotor, 300);
		}
		else if (getMotorEncoder(irMotor) < -180)
		{
			turnRight(irMotor, 300);
		}
		if(getIRBeaconDirection(infrared) > threshold)
		{
			turnRight(irMotor, rotationAmount);
		}
		else if (getIRBeaconDirection(infrared) < -threshold)
		{
			turnLeft(irMotor, rotationAmount);
		}
		else
		{
			setMotorSpeed(irMotor, 0);
			return getIRBeaconDirection(infrared);
		}
	}
	while(getIRBeaconStrength(infrared) > followingDistance)
	{
		int irDir = trackIRDir(5);
		displayTextLine(7, "%d", irDir);
		turnVehicleRight(irDir);
		moveVehicle(20);
	}
	return 's';
}
char still()
{
	if(getIRBeaconStrength(infrared) > followingDistance)
	{
		return 't';
	}
	if(lightSensed())
	{
		return 'o';
	}
	return 's';
}
char takeOutTrash()
{
	turnVehicleRight(180);
	moveVehicle(360);
	turnVehicleRight(180);

	return 'd';
}
char dumping()
{
	if(!lightSensed())
	{
		return 't';
	}
	return 'd';
}
task kill()
{
	if(	getIRRemoteButtons(infrared) != killCode)
	{
		stopAllTasks();
		resetMotor(irMotor);
		resetMotor(usMotor);
	}
}
task displayData()
{
	while(true)
	{
		displayTextLine(0, "Remote: %d", getIRRemoteButtons(infrared));
		displayTextLine(1, "Distance: %d", getIRBeaconStrength(infrared));
		displayTextLine(2, "Rotation: %d", getMotorEncoder(irMotor));
		displayTextLine(5, "Color: %d", SensorValue(color));
	}
}
task main()
{
	// reset all motors
	//resetMotor(irMotor);
	//resetMotor(usMotor);

	startTask(displayData);

	while(getIRRemoteButtons(infrared) != killCode)
	{
	
		/*if(state == 's')
		{
			state = still();
		}
		else if (state == 't')
		{
			*/turnVehicleRight(trackIRDir(5));/*
		}
		else if (state == 'd')
		{
			state = dumping();
		}
		else if (state == 'o')
		{
			state = takeOutTrash();
		}*/
	}
		//stopTask(displayData);
		//reset all motors for next use
		//resetMotor(irMotor);
		//resetMotor(usMotor);

}

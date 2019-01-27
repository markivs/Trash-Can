
task main()
{

	while(true)
	{
		setMotorTarget(motorA, 10000, 100);
		displayTextLine(0, "Color Sensor: %d", SensorValue(S1));
	}
}

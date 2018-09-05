//======================================================authorship
//CC-BY Michael Osthege (2013)
//https://code.msdn.microsoft.com/windowsapps/Bluetooth-communication-7130c260
//======================================================
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using TCD.Controls;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using TCD.Arduino.Bluetooth;
using Windows.UI.Popups;
using WindowsPreview.Kinect;
using Microsoft.Kinect.VisualGestureBuilder;

namespace BluetoothCommunicationSampleController
{

    /* Before you can connect a Bluetooth device to Windows (Phone) 8, you have to go to the Bluetooth settings
     * (Win+I\PC Settings\Devices\Bluetooth) and pair the device. Probably you have to enter a PIN, which is often 1234
     * After you've paired the device, you can launch the BluetoothCommunicationSampleController app and tap "Connect"
     */

    public sealed partial class MainPage : Page
    {
        private KinectSensor kinectSensor = null;
        VisualGestureBuilderDatabase gestureDatabase = null;
        VisualGestureBuilderFrameSource vgbFrameSource = null;
        VisualGestureBuilderFrameReader vgbFrameReader = null;

        Gesture move_left;
        Gesture move_right;
        Gesture move_up;
        Gesture move_down;

        BodyFrameReader bodyFrameReader;

        public MainPage()
        {
            this.InitializeComponent();
            this.Initialize();
            App.BluetoothManager.ExceptionOccured += BluetoothManager_ExceptionOccured;
        }
        protected override void OnNavigatedFrom(Windows.UI.Xaml.Navigation.NavigationEventArgs e)
        {
            App.BluetoothManager.Disconnect();
            //clean up
        }

        #region Bluetooth Connection Lifecycle
        //control
        private async void BluetoothConnect_Click(object sender, RoutedEventArgs e)
        {
            //ask the user to connect 
            await App.BluetoothManager.EnumerateDevicesAsync((sender as Button).GetElementRect());
            //displays a PopupMenu above the ConnectButton - uses GetElementRect() from TCD.Controls to determine the area
        }
        private async void BluetoothManager_ExceptionOccured(object sender, Exception ex)
        {
            var md = new MessageDialog(ex.Message, "We've got a problem with bluetooth...");
            md.Commands.Add(new UICommand("Ah.. thanks.."));
            md.DefaultCommandIndex = 0;
            var result = await md.ShowAsync();
        }
        #endregion

        #region Send Data to Arduino
        private async void kinect_arduino(string message)
        {
            await App.BluetoothManager.SendMessageAsync(message);
        }
        #endregion

        #region Get Data from Kinect
        // referenced from: http://kinect.github.io/tutorial/lab12/index.html
        // referenced from: https://www.youtube.com/watch?v=OrDTekg01sk
        void Initialize()
        {
            kinectSensor = KinectSensor.GetDefault();

            if (kinectSensor == null)
            {
                throw new ArgumentNullException("kinectSensor");
            }

            bodyFrameReader = kinectSensor.BodyFrameSource.OpenReader();

            if (bodyFrameReader == null)
            {
                throw new ArgumentNullException("bodyFrameReader");
            }

            bodyFrameReader.FrameArrived += bodyFrameReader_FrameArrived;

            gestureDatabase = new VisualGestureBuilderDatabase(@"Database\Gestures.gbd");

            // create the vgb source. The associated body tracking ID will be set when a valid body frame arrives from the sensor.
            this.vgbFrameSource = new VisualGestureBuilderFrameSource(kinectSensor, 0);

            foreach (Gesture gesture in gestureDatabase.AvailableGestures)
            {
                if (gesture.Name.Equals("Move_Left"))
                {
                    move_left = gesture;
                    this.vgbFrameSource.AddGesture(move_left);
                }
                if (gesture.Name.Equals("Move_Right"))
                {
                    move_right = gesture;
                    this.vgbFrameSource.AddGesture(move_right);
                }
                if (gesture.Name.Equals("Jump"))
                {
                    move_up = gesture;
                    this.vgbFrameSource.AddGesture(move_up);
                }
                if (gesture.Name.Equals("Down"))
                {
                    move_down = gesture;
                    this.vgbFrameSource.AddGesture(move_down);
                }
            }

            vgbFrameReader = vgbFrameSource.OpenReader();
            vgbFrameReader.FrameArrived += vgbFrameReader_FrameArrived;
            kinectSensor.Open();
        }

        void bodyFrameReader_FrameArrived(object sender, BodyFrameArrivedEventArgs e)
        {
            if (!vgbFrameSource.IsTrackingIdValid)
            {
                using (BodyFrame bodyFrame = e.FrameReference.AcquireFrame())
                {
                    if (bodyFrame != null)
                    {
                        Body[] bodies = new Body[6];
                        bodyFrame.GetAndRefreshBodyData(bodies);
                        Body closestBody = null;
                        foreach (Body b in bodies)
                        {
                            if (b.IsTracked)
                            {
                                if (closestBody == null)
                                {
                                    closestBody = b;
                                }
                                else
                                {
                                    Joint newHeadJoint = b.Joints[JointType.Head];
                                    Joint oldHeadJoint = closestBody.Joints[JointType.Head];
                                    if (newHeadJoint.TrackingState == TrackingState.Tracked && newHeadJoint.Position.Z < oldHeadJoint.Position.Z)
                                    {
                                        closestBody = b;
                                    }
                                }
                            }
                        }
                        if (closestBody != null)
                        {
                            vgbFrameSource.TrackingId = closestBody.TrackingId;
                        }
                    }
                }
            }
        }

        void vgbFrameReader_FrameArrived(object sender, VisualGestureBuilderFrameArrivedEventArgs e)
        {
            using (var frame = e.FrameReference.AcquireFrame())
            {
                if (frame != null)
                {

                    IReadOnlyDictionary<Gesture, DiscreteGestureResult> discreteResults = frame.DiscreteGestureResults;

                    if (discreteResults != null)
                    {
                        String bluetooth_char = null;
                        String old_bc = null;
 
                        foreach (Gesture gesture in this.vgbFrameSource.Gestures)
                        {
                            if (gesture.Name.Equals("Move_Left"))
                            {
                                DiscreteGestureResult result = null;
                                discreteResults.TryGetValue(gesture, out result);

                                if (result != null)
                                {
                                    if (result.Detected)
                                    {
                                        bluetooth_char = "l";
                                    }
                                }
                            }
                            if (gesture.Name.Equals("Move_Right"))
                            {
                                DiscreteGestureResult result = null;
                                discreteResults.TryGetValue(gesture, out result);

                                if (result != null)
                                {
                                    if (result.Detected)
                                    {
                                        bluetooth_char = "r";
                                    }
                                }
                            }
                            if (gesture.Name.Equals("Jump"))
                            {
                                DiscreteGestureResult result = null;
                                discreteResults.TryGetValue(gesture, out result);

                                if (result != null)
                                {
                                    if (result.Detected)
                                    {
                                        bluetooth_char = "u";
                                    }
                                }
                            }
                            if (gesture.Name.Equals("Down"))
                            {
                                DiscreteGestureResult result = null;
                                discreteResults.TryGetValue(gesture, out result);

                                if (result != null)
                                {
                                    if (result.Detected)
                                    {
                                        bluetooth_char = "d";
                                    }
                                }
                            }
                        }

                        if (!(bluetooth_char == old_bc))
                        {
                            // only send char over bluetooth if it is different from last one sent
                            kinect_arduino(bluetooth_char);
                            old_bc = bluetooth_char;
                        }
                    }
                }
            }
        }
        #endregion
    }
}
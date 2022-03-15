# Essentials API
You can find detailed information about usage of Essentials API.

## Getting started
First of all, Essentials requires Dragonade to run and compile. If your `scripts` project is not Dragonade, you will have errors.  
To use API provided by Essentials, you need to clone Essentials into your solution and add the Essentials project in your project as a Dependency.  
First you need to clone Essentials into your solution, you can use Git or download the source code as Zip, and add it in your solution.  
<br />
![image](https://user-images.githubusercontent.com/44943820/158345017-35d4840c-dc1c-44a9-a412-a9b39cf1121e.png)  
![image](https://user-images.githubusercontent.com/44943820/158345154-a28dd174-8df7-4804-8901-06f9915ae129.png)  
<br />
Then in your IDE (in this case Visual Studio) right click on `References` under your project tree, then click `Add Reference...`.  
<br />
![image](https://user-images.githubusercontent.com/44943820/158338973-3ce21df3-2fcb-430e-8f95-d9dc4439920e.png)  
<br />
In the opened dialog, tick `Essentials`. Do not untick other options as these are required for a Dragonade plugin.  
<br />
![image](https://user-images.githubusercontent.com/44943820/158339639-28c048d2-9d7f-4d52-8da0-fb3b1e666ed9.png)  
<br />
But we're not done yet. Now open your project properties. (Select your project in Solution Explorer, and hit Alt+Enter)  
In the dialog, go to C/C++ > General and `<Edit...>` Additional Include Directories.  
<br />
![image](https://user-images.githubusercontent.com/44943820/158342494-95ef178a-6061-4fe7-bb7d-4e1996b9c99d.png)  
<br />
Click on Add button, and put `..\Essentials` into the new item.  
<br />
![image](https://user-images.githubusercontent.com/44943820/158344368-6562f83e-4b22-46d0-a57d-88a53e90c697.png)  
<br />
Press OK, and save it.

That's all! Now you can include headers from Essentials project and use it's API from your plugin.

## What's next?
You can find documentation of Holograms API [here](holograms.md).

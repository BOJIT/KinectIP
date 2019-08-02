<!DOCTYPE html>
<html>

<!--Error Reporting PHP-->
<?php
    error_reporting(E_ALL);
    ini_set('display_errors', '1');
?>

<!--Head-->
<head>
  <title>Kinect-IP Control Panel</title>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <meta http-equiv="Content-Type" content="text/html; charset=utf-8">
  <meta http-equiv="X-UA-Compatible" content="IE=edge,chrome=1">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <meta name="apple-mobile-web-app-capable" content="yes">
  <meta name="apple-mobile-web-app-status-bar-style" content="black-translucent">
  <link rel="stylesheet" href="styles/w3.css">
  <link rel="stylesheet" href="styles/w3-theme-teal.css">
  <link rel="stylesheet" href="styles/font-awesome.css">
  <style>
    html, body, h1, h2, h3, h4, h5, h6 {
      font-family: "Trebuchet MS", cursive, sans-serif;
    }
    #parentDiv {
      position: relative;
    }

    #parentDiv .childDiv {
      position: absolute;
      bottom: 0;
      left: 0;
    }
  </style>
</head>
<body onload="document.getElementById('id01').style.display='block'">

<!--PHP Functions-->
<?php
  function createDefaults() {
    // function only called if defaults file is not present - this way users can
    // change default settings without requiring the source-code
    $filename = "config/defaults.conf";
    if(file_exists($filename) == FALSE) {
      $config = fopen($filename, "w+") or die("Unable to open file!"); 
      $frequency = "Frequency=10";
      fwrite($config, $frequency."\n");
      $hostname = "Hostname=KinectIP";
      fwrite($config, $hostname."\n");
      $IP = "IP=DHCP";
      fwrite($config, $IP."\n");
      $Address = "Address=192.168.1.100";
      fwrite($config, $Address."\n");
      $Stream_RGB = "Stream_RGB=ON";
      fwrite($config, $Stream_RGB."\n");
      $Stream_DEPTH = "Stream_DEPTH=OFF";
      fwrite($config, $Stream_DEPTH."\n");
      $Stream_INDEX = "Stream_INDEX=OFF";
      fwrite($config, $Stream_INDEX."\n");
      $Stream_POINTS = "Stream_POINTS=OFF";
      fwrite($config, $Stream_POINTS."\n");
      $RGB_RES = "RGB_RES=1080p";
      fwrite($config, $RGB_RES."\n");
      $DEPTH_RES = "DEPTH_RES=720p";
      fwrite($config, $DEPTH_RES."\n");
      $INDEX_RES = "INDEX_RES=720p";
      fwrite($config, $INDEX_RES."\n");
      $RGB_FRAME = "RGB_FRAME=25p";
      fwrite($config, $RGB_FRAME."\n");
      $DEPTH_FRAME = "DEPTH_FRAME=25p";
      fwrite($config, $DEPTH_FRAME."\n");
      $INDEX_FRAME = "INDEX_FRAME=25p";
      fwrite($config, $INDEX_FRAME."\n");
      $POINTS_FRAME = "POINTS_FRAME=25p";
      fwrite($config, $POINTS_FRAME."\n");
      fclose($config);
    }  
  }
  
  function readConfig($line) {
    $filename = "config/settings.conf";
    $config = fopen($filename, "r");
    for($i=0;$i<$line - 1;$i++) {
      fgets($config);
    }
    $field = fgets($config);
    fclose($config);
    $parsed_field = trim(substr($field, strpos($field, "=") + 1));
    return $parsed_field;
  }

  function InitialiseSHMEM() {
        //----- SHARED MEMORY CONFIGURATION -----
    $SEMAPHORE_KEY = 291623581;   			//Semaphore unique key (MAKE DIFFERENT TO C++ KEY)
    $SHARED_MEMORY_KEY = 672213396;   	//Shared memory unique key (SAME AS C++ KEY)

    //Create the semaphore
    $semaphore_id = sem_get($SEMAPHORE_KEY, 1);		//Creates, or gets if already present, a semaphore
    if ($semaphore_id === false)
    {
        echo "Failed to create semaphore.  Reason: $php_errormsg<br />";
        exit;
    }

    //Acquire the semaphore
    if (!sem_acquire($semaphore_id))						//If not available this will stall until the semaphore is released by the other process
    {
        echo "Failed to acquire semaphore $semaphore_id<br />";
        sem_remove($semaphore_id);						//Use even if we didn't create the semaphore as something has gone wrong and its usually debugging so lets no lock up this semaphore key
        exit;
    }

    //We have exclusive access to the shared memory (the other process is unable to aquire the semaphore until we release it)

    //Setup access to the shared memory
    $shared_memory_id = shmop_open($SHARED_MEMORY_KEY, "w", 0, 0);	//Shared memory key, flags, permissions, size (permissions & size are 0 to open an existing memory segment)
                                                                    //flags: "a" open an existing shared memory segment for read only, "w" read and write to a shared memory segment
    if (empty($shared_memory_id))
    {
      echo "Failed to open shared memory.<br />";			//<<<< THIS WILL HAPPEN IF THE C APPLICATION HASN'T CREATED THE SHARED MEMORY OR IF IT HAS BEEN SHUTDOWN AND DELETED THE SHARED MEMORY
    }
    else
    {
      //--------------------------------------------
      //----- READ AND WRITE THE SHARED MEMORY -----
      //--------------------------------------------
      echo "Shared memory size: ".shmop_size($shared_memory_id)." bytes<br />";

      //----- READ FROM THE SHARED MEMORY -----
      $shared_memory_string = shmop_read($shared_memory_id, 0, 8);				//Shared memory ID, Start Index, Number of bytes to read
      if($shared_memory_string == FALSE) 
      {
          echo "Failed to read shared memory";
          sem_release($semaphore_id);
          exit;
      }

      $shared_memory_array = array_slice(unpack('C*', "\0".$shared_memory_string), 1);

      echo "Shared memory bytes: ";
      for($i = 0; $i < 8; $i++)
      {
        echo $shared_memory_array[$i] . ", ";
      }
      echo "<br />";

      //----- WRITE TO THE SHARED MEMORY -----
      if(isset($_REQUEST['shutdown']))			//Include "?shutdown" at the end of the url to write these bytes which causes the C application to exit
      {
        //The array to write
        $shared_memory_array = array(30, 255);  // testing purposes
        
        //Convert the array of byte values to a byte string
        $shared_memory_string = call_user_func_array(pack, array_merge(array("C*"), $shared_memory_array));
        echo "Writing bytes: $shared_memory_string<br />";
        shmop_write($shared_memory_id, $shared_memory_string, 8);			//Shared memory id, string to write, Index to start writing from
                                                                      //Note that a trailing null 0x00 byte is not written, just the byte values / characters, so in this example just 2 bytes are written.
      }
                                                                  
      //Detach from the shared memory
      shmop_close($shared_memory_id);
    }

    //Release the semaphore
    if (!sem_release($semaphore_id))				//Must be called after sem_acquire() so that another process can acquire the semaphore
        echo "Failed to release $semaphore_id semaphore<br />";
  }

  function ReadSHMEM($index) {
    //----- SHARED MEMORY CONFIGURATION -----
    $SEMAPHORE_KEY = 291623581;   			//Semaphore unique key (MAKE DIFFERENT TO C++ KEY)
    $SHARED_MEMORY_KEY = 672213396;   	//Shared memory unique key (SAME AS C++ KEY)

    //Create the semaphore
    $semaphore_id = sem_get($SEMAPHORE_KEY, 1);		//Creates, or gets if already present, a semaphore
    if ($semaphore_id === false)
    {
        echo "Failed to create semaphore.  Reason: $php_errormsg<br />";
        exit;
    }

    //Acquire the semaphore
    if (!sem_acquire($semaphore_id))						//If not available this will stall until the semaphore is released by the other process
    {
        echo "Failed to acquire semaphore $semaphore_id<br />";
        sem_remove($semaphore_id);						//Use even if we didn't create the semaphore as something has gone wrong and its usually debugging so lets no lock up this semaphore key
        exit;
    }

    //We have exclusive access to the shared memory (the other process is unable to aquire the semaphore until we release it)

    //Setup access to the shared memory
    $shared_memory_id = shmop_open($SHARED_MEMORY_KEY, "w", 0, 0);	//Shared memory key, flags, permissions, size (permissions & size are 0 to open an existing memory segment)
                                                                    //flags: "a" open an existing shared memory segment for read only, "w" read and write to a shared memory segment
    if (empty($shared_memory_id))
    {
      echo "Failed to open shared memory.<br />";			//<<<< THIS WILL HAPPEN IF THE C APPLICATION HASN'T CREATED THE SHARED MEMORY OR IF IT HAS BEEN SHUTDOWN AND DELETED THE SHARED MEMORY
    }
    else
    {
      //----- READ FROM THE SHARED MEMORY -----
      $shared_memory_string = shmop_read($shared_memory_id, 0, 8);				//Shared memory ID, Start Index, Number of bytes to read
      if($shared_memory_string == FALSE) 
      {
          echo "Failed to read shared memory";
          sem_release($semaphore_id);
          exit;
      }

      $shared_memory_array = array_slice(unpack('C*', "\0".$shared_memory_string), 1);

      return $shared_memory_array[$index];
                                                          
      //Detach from the shared memory
      shmop_close($shared_memory_id);
    }

    //Release the semaphore
    if (!sem_release($semaphore_id))				//Must be called after sem_acquire() so that another process can acquire the semaphore
        echo "Failed to release $semaphore_id semaphore<br />";
  }

  //End of Functions

  //InitialiseSHMEM();   //Called to get initial status

  echo(ReadSHMEM(2));

  createDefaults();   //If no defaults.config is provided, one will be generated

  $defaults = "config/defaults.conf";
  $settings = 'config/settings.conf';

  if(file_exists($settings) == FALSE) {   //Source Repository does not contain settings, these are generated from
    if (!copy($defaults, $settings)) {    //defaults.config the first time the webserver is loaded
        echo "failed to generate settings...\n";
    }
  }
?>

<!-- Header -->
<header class="w3-container w3-theme w3-padding-0" id="myHeader">
  <div class="w3-third w3-padding-0"> 
  <a href="https://github.com/BOJIT" target="_blank"><img src="images/BOJIT_Logo.png" alt="BOJIT" 
  style="width:100px;height:75px;" class="w3-animate-bottom w3-align-right"></a>
  </div>
  <div class="w3-third w3-padding-0 w3-center">
    <h2 class="w3-xxlarge w3-animate-bottom w3-padding-0">KINECT-IP CONTROL PANEL</h2>   
  </div>
  <div class="w3-third w3-padding-0">  
    <br>
    <!-- Tab Menu -->
    <div class="w3-bar w3-theme">
      <button class="w3-bar-item w3-button testbtn w3-padding-16 w3-right" onclick="openTab(event,'Stream')">Stream</button>
      <button class="w3-bar-item w3-button testbtn w3-padding-16 w3-right" onclick="openTab(event,'Calibrate')">Calibrate</button>
      <button class="w3-bar-item w3-button testbtn w3-padding-16 w3-right" onclick="openTab(event,'About')">About</button>
      <button class="w3-bar-item w3-button testbtn w3-padding-16 w3-right" onclick="openTab(event,'Manual')">Manual</button>
    </div>
  </div>
</header>

<!-- Stream Tab -->
<div id="Stream" class="w3-container tab w3-animate-opacity">
  <div class="w3-center">
    <p></p>
  </div>
  <div class="w3-row-padding w3-cell-row">
    <!--Left Tile-->
    <form class="w3-container w3-cell w3-mobile w3-card-4" id="parentDiv" 
    style="width:49%" action="form_handler.php" method="post">
      <div class="w3-half">
        <h2>Configuration:</h2>
      </div>
      <div class="w3-half">
        <button type="submit" name="Action" value="Default" class="w3-right w3-btn w3-section w3-ripple w3-padding">
        <u>[Restore Defaults]</u></button>
      </div>
      <div class="w3-section">
        <div class="w3-half">
          <label>Hostname:</label>    
          <input class="w3-input" type="text" name="Hostname" placeholder="<?php echo(readConfig(2)); ?>" >
        </div>
        <div class="w3-half">
          <div class="w3-margin-left">
            <label>IP:</label><br>
            <input class="w3-radio" type="radio" name="IP" onchange="showIP()" 
            value="DHCP" <?php if (readConfig(3) == 'DHCP') {echo 'checked';} ?>>
            <label>DHCP</label>
            <input class="w3-radio" type="radio" name="IP" id="StaticIP" onchange="showIP()" 
            value="Static" <?php if (readConfig(3) == 'Static'){echo 'checked';} ?>>
            <label>Static</label>
          </div>
        </div>
      </div>
      <div class="w3-row">
        <div class="w3-section w3-animate-opacity" id="IPBox" style="display:none">
          <label>IP Address:</label>   
          <input class="w3-input" name="Address" id="IPEntry" type="text" minlength="7" maxlength="15" size="15" 
          pattern="^((\d{1,2}|1\d\d|2[0-4]\d|25[0-5])\.){3}(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])$" <?php if (readConfig(3) == 'Static')
          {echo 'placeholder='.readConfig(4);} else {echo 'placeholder="xxx.xxx.xxx.xxx"';} ?>>
        </div>
        <div class="w3-section"> 
          <label>Active Streams:</label> 
          <div class="w3-row-padding">
            <div class="w3-quarter">
              <div class="w3-center">
                <input type='hidden' name='Stream_RGB' value='OFF' id="RGB0" checked>
                <label>
                  <input class="w3-check" type="checkbox" style="display:none" name="Stream_RGB" id="RGB" value="ON"
                  <?php if (readConfig(5) == 'ON') {echo 'checked';} ?> onchange="changeColor('RGB')">
                  <span class="checkmark w3-btn w3-block w3-section w3-dark-grey w3-ripple w3-padding" id="RGB1">RGB</span>
                </label>
                <div id="RGB2" class="w3-section w3-hide w3-animate-opacity">
                  <select class="w3-select" name="RGB_RES">
                    <option value="480p" <?php if (readConfig(9) == '480p'){echo 'selected';} ?>>480p</option>
                    <option value="720p" <?php if (readConfig(9) == '720p'){echo 'selected';} ?>>720p</option>
                    <option value="1080p" <?php if (readConfig(9) == '1080p'){echo 'selected';} ?>>1080p</option>
                  </select>
                  <select class="w3-select" name="RGB_FRAME">
                    <option value="24p" <?php if (readConfig(12) == '24p'){echo 'selected';} ?>>24p</option>
                    <option value="25p" <?php if (readConfig(12) == '25p'){echo 'selected';} ?>>25p</option>
                    <option value="50i" <?php if (readConfig(12) == '50i'){echo 'selected';} ?>>50i</option>
                    <option value="60i" <?php if (readConfig(12) == '60i'){echo 'selected';} ?>>60i</option>
                  </select>
                </div>
              </div>
            </div>
            <div class="w3-quarter">
              <div class="w3-center">
                <input type='hidden' name='Stream_DEPTH' value='OFF' id="DEPTH0" checked>
                <label>
                  <input class="w3-check" type="checkbox" style="display:none" name="Stream_DEPTH" id="DEPTH" value="ON"
                  <?php if (readConfig(6) == 'ON') {echo 'checked';} ?> onchange="changeColor('DEPTH')">
                  <span class="checkmark w3-btn w3-block w3-section w3-dark-grey w3-ripple w3-padding" id="DEPTH1">DEPTH</span>
                </label>
                <div id="DEPTH2" class="w3-section w3-hide w3-animate-opacity">
                  <select class="w3-select" name="DEPTH_RES">
                    <option value="480p" <?php if (readConfig(10) == '480p'){echo 'selected';} ?>>480p</option>
                    <option value="720p" <?php if (readConfig(10) == '720p'){echo 'selected';} ?>>720p</option>
                    <option value="1080p" <?php if (readConfig(10) == '1080p'){echo 'selected';} ?>>1080p</option>
                  </select>
                  <select class="w3-select" name="DEPTH_FRAME">
                    <option value="24p" <?php if (readConfig(13) == '24p'){echo 'selected';} ?>>24p</option>
                    <option value="25p" <?php if (readConfig(13) == '25p'){echo 'selected';} ?>>25p</option>
                    <option value="50i" <?php if (readConfig(13) == '50i'){echo 'selected';} ?>>50i</option>
                    <option value="60i" <?php if (readConfig(13) == '60i'){echo 'selected';} ?>>60i</option>
                  </select>
                </div>
              </div>
            </div>
            <div class="w3-quarter">
              <div class="w3-center">
                <input type='hidden' name='Stream_INDEX' value='OFF' id="INDEX0" checked>
                <label>
                  <input class="w3-check" type="checkbox" style="display:none" name="Stream_INDEX" id="INDEX" value="ON"
                  <?php if (readConfig(7) == 'ON') {echo 'checked';} ?> onchange="changeColor('INDEX')">
                  <span class="checkmark w3-btn w3-block w3-section w3-dark-grey w3-ripple w3-padding" id="INDEX1">INDEX</span>
                </label>
                <div id="INDEX2" class="w3-section w3-hide w3-animate-opacity">
                  <select class="w3-select" name="INDEX_RES">
                    <option value="480p" <?php if (readConfig(11) == '480p'){echo 'selected';} ?>>480p</option>
                    <option value="720p" <?php if (readConfig(11) == '720p'){echo 'selected';} ?>>720p</option>
                    <option value="1080p" <?php if (readConfig(11) == '1080p'){echo 'selected';} ?>>1080p</option>
                  </select>
                  <select class="w3-select" name="INDEX_FRAME">
                    <option value="24p" <?php if (readConfig(14) == '24p'){echo 'selected';} ?>>24p</option>
                    <option value="25p" <?php if (readConfig(14) == '25p'){echo 'selected';} ?>>25p</option>
                    <option value="50i" <?php if (readConfig(14) == '50i'){echo 'selected';} ?>>50i</option>
                    <option value="60i" <?php if (readConfig(14) == '60i'){echo 'selected';} ?>>60i</option>
                  </select>
                </div>
              </div>
            </div>
            <div class="w3-quarter">
              <div class="w3-center">
                <input type='hidden' name='Stream_POINTS' value='OFF' id="POINTS0" checked>
                <label>
                  <input class="w3-check" type="checkbox" style="display:none" name="Stream_POINTS" id="POINTS" value="ON"
                  <?php if (readConfig(8) == 'ON') {echo 'checked';} ?> onchange="changeColor('POINTS')">
                  <span class="checkmark w3-btn w3-block w3-section w3-dark-grey w3-ripple w3-padding" id="POINTS1">POINTS</span>
                </label>
                <div id="POINTS2" class="w3-section w3-hide w3-animate-opacity">
                  <select class="w3-select" name="POINTS_FRAME">
                    <option value="24p" <?php if (readConfig(12) == '24p'){echo 'selected';} ?>>24p</option>
                    <option value="25p" <?php if (readConfig(12) == '25p'){echo 'selected';} ?>>25p</option>
                    <option value="50i" <?php if (readConfig(12) == '50i'){echo 'selected';} ?>>50i</option>
                    <option value="60i" <?php if (readConfig(12) == '60i'){echo 'selected';} ?>>60i</option>
                  </select>
                </div>
              </div>
            </div>
          </div>
        </div>
        <br><br>
      </div>
      <div class="childDiv w3-padding-large w3-block">
        <input type="submit"  name="Action" value="Update Settings"
        class="w3-button w3-block w3-section w3-teal w3-ripple">
      </div>
    </form>
    <!--Centre Gap-->
    <div class="w3-cell w3-mobile" style="width:2%">
      <p></p>
    </div>
    <!--Right Tile-->
    <div class="w3-container w3-cell w3-mobile w3-card-4" style="width:49%">
      <h2>Status:</h2>
      <ul class="w3-ul w3-margin-bottom">
      <div class="w3-row-padding">
        <div class="w3-third w3-center">
          <label>
            <input class="w3-check" type="checkbox" style="display:none" name="Status" id="Status" value="Status">
            <span class="checkmark w3-btn w3-block w3-section w3-dark-grey w3-ripple w3-padding" 
            onclick="changeColor('Status')" id="Status1">Stream Active</span>
          </label>
        </div>
        <div class="w3-third w3-center">
          <label>
            <input class="w3-check" type="checkbox" style="display:none" name="Identify" id="Identify" value="Identify">
            <span class="checkmark w3-btn w3-block w3-section w3-dark-grey w3-ripple w3-padding" 
            onclick="changeColor('Identify')" id="Identify1">Identify</span>
          </label>
        </div>
        <div class="w3-third w3-center">
          <button class="w3-btn w3-block w3-section w3-red w3-ripple w3-padding"
          name="Power Off" onclick="document.getElementById('id02').style.display='block'" id="Power Off" value="Power Off">Power Off</button>
        </div>
      </div>
      <br>
      <div class="w3-row-padding">
        <ul class="w3-ul">
          <li class="w3-theme w3-display-container">Performance Monitor: <button class="w3-button w3-display-right w3-ripple">
          <a href="http://<?php echo(gethostname());?>.local:19999/#;theme=slate;help=true" target="_blank">Full Telemetry Here:</a></button></li>
        </ul>
        <iframe src="telemetry.php" id="telemetry" class="w3-block" frameBorder="0">Data Cannot Be Loaded</iframe>
        <br>
      </div>
    </div>
  </div>
</div>

<!-- Calibrate Tab -->
<div id="Calibrate" class="w3-container tab w3-animate-opacity">
  <h2>Calibrate</h2>
</div>

<!-- Manual Tab -->
<div id="Manual" class="w3-container tab w3-animate-opacity">
  <div class="w3-center">
    <h2>Manual</h2>
  </div>
  <div class="w3-row-padding w3-center w3-margin-top">
    <div class="w3-third">
      <div class="w3-card w3-container" style="min-height:460px">
        <h3>Responsive</h3><br>
        <p>Built-in responsiveness</p>
        <p>Mobile first fluid grid</p>
        <p>Fits any screen sizes</p>
        <p>PC Tablet and Mobile</p>
      </div>
    </div>

    <div class="w3-third">
      <div class="w3-card w3-container" style="min-height:460px">
        <h3>Standard CSS</h3><br>
        <p>Standard CSS only</p>
        <p>Easy to learn</p>
        <p>No need for jQuery</p>
        <p>No JavaScript library</p>
      </div>
    </div>

    <div class="w3-third">
      <div class="w3-card w3-container" style="min-height:460px">
        <h3>Design</h3><br>
        <p>Paper like design</p>
        <p>Bold colors and shadows</p>
        <p>Equal across platforms</p>
        <p>Equal across devices</p>
      </div>
    </div>
  </div>
</div>

<!-- About Tab -->
<div id="About" class="w3-container tab w3-animate-opacity">
  <h2>About</h2>
  <p>Note: Password Protection is not encrypted, do not put secure passwords here! 
  This is largely meant as an anti-tamper measure for using the Kinect-IP on public networks 
  or stopping other technicians messing with the product settings rather than a 
  highly secure login page.</p>
  <p>Sources: Netdata, w3.css, libfreenect_v2, NDI w. Link</p>
</div>

<!-- Footer -->
<div class="w3-center">
    <p></p>
</div>
<footer class="w3-container w3-theme-dark w3-padding-16">
  <div class="w3-half"> 
    <h6>&#169 BOJIT 2019</h6>
  </div>
  <div class="w3-half w3-padding-small"> 
    <button class="w3-btn w3-medium w3-dark-grey w3-hover-light-grey" style="float:right" 
    onclick="document.getElementById('id01').style.display='block'" style="font-weight:900;">LOGOUT</button>
  </div>
</footer>

<!-- Authentification -->
<div id="id01" class="w3-modal">
    <div class="w3-modal-content w3-card-4 w3-animate-top">
      <header class="w3-container w3-teal"> 
        <h4>Authentification Required</h4>
      </header>
      <div class="w3-padding">
          <h5>Enter Password: <input name="password" id="password" type="password">
          <button onclick="Password()" id="enterPassword" type="button">Enter</button>
          <span id="incorrect" style="display:none"><span class="w3-margin-left" 
          style="color:red">Incorrect Password</span></span></h5>
          <p>Product Manual Available as PDF <a target="_blank" href="setup.README">HERE</a></p>
      </div>
      <footer class="w3-container w3-teal">
        <p></p>
      </footer>
    </div>
</div>

<!-- Shutdown -->
<div id="id02" class="w3-modal">
    <div class="w3-modal-content w3-card-4 w3-animate-top">
      <header class="w3-container w3-teal"> 
        <h4>Shutdown Confirmation</h4>
        <span onclick="document.getElementById('id02').style.display='none'"
        class="w3-button w3-display-topright">×</span>
      </header>
      <div class="w3-padding w3-center">
        <button class="w3-btn w3-block w3-section w3-red w3-ripple w3-padding-large"
        name="Power Off" onclick="window.location.href = 'shutdown.php';" value="Power Off">Power Off</button>
        <p class="w3-center">Unit can be restarted by pressing the power button on the case</p>
      </div>
      <footer class="w3-container w3-teal">
        <p></p>
      </footer>
    </div>
</div>

<!-- Scripts-->
<script type = "text/javascript" src = "script.js" ></script>

</body>
</html>
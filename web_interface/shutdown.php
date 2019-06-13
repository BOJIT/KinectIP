<!DOCTYPE html>
<html>
<!--Head-->
<head>
  <title>Kinect-IP Control Panel</title>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="styles/w3.css">
  <link rel="stylesheet" href="styles/w3-theme-teal.css">
  <link rel="stylesheet" href="styles/font-awesome.css">
  <style>
    html, body, h1, h2, h3, h4, h5, h6 {
      font-family: "Trebuchet MS", cursive, sans-serif;
    }

    html, body {
      height: 100%;
    }

    .full-height {
      height: 100%;
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

<body>
    <div class="w3-center w3-container w3-padding-large w3-theme w3-display-middle">
        <h1>SHUTTING DOWN...</h1>
        <h4>Do not unplug until power<br> LED is no longer lit</h4>
    </div>
    <?php
        system('sudo /sbin/shutdown -h now > /dev/null 2>&1 &');
    ?>

<!-- Script for Sidebar, Tabs, Accordions, Progress bars and slideshows -->
<script type = "text/javascript" src = "script.js" ></script>

</body>
</html>
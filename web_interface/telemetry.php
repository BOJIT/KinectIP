<!DOCTYPE html>
<html>

<!--Head-->
<head>
    <title>Embedded Telemetry</title>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <meta http-equiv="Content-Type" content="text/html; charset=utf-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge,chrome=1">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <meta name="apple-mobile-web-app-capable" content="yes">
    <meta name="apple-mobile-web-app-status-bar-style" content="black-translucent">
    <meta name="application-name" content="netdata">
    <meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
    <meta charset="utf-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge,chrome=1">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <meta name="apple-mobile-web-app-capable" content="yes">
    <meta name="apple-mobile-web-app-status-bar-style" content="black-translucent">
    <meta name="author" content="costa@tsaousis.gr">
    <meta property="og:locale" content="en_US" />
    <meta property="og:type" content="website"/>
    <meta property="og:site_name" content="netdata"/>
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
    <script type="text/javascript" src="http://<?php echo(gethostname());?>.local:19999/dashboard.js"></script>
</head>
<body>
    <div class="w3-block">
        <div data-netdata="system.cpu"
        data-chart-library="sparkline"
        data-width="100%"
        data-height="50"
        data-after="-30"
        ></div>
        <div data-netdata="system.cpu"
        data-chart-library="dygraph"
        data-dygraph-theme="sparkline"
        data-legend="no"
        data-width="100%"
        data-height="50"
        data-after="-300"
        ></div>
</body>
</html>

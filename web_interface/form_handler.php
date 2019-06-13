<!DOCTYPE html>
<html>
<body>

<?php
    function restoreDefaults() {
        $defaults = "config/defaults.conf";
        $settings = 'config/settings.conf';
    
        if (!copy($defaults, $settings)) {
            echo "failed to reset factory defaults...\n";
        }
    }

    function storePrevious() {
        $filename = "config/settings.conf";
        $filename_temp = $filename.".tmp";
        if (!copy($filename, $filename_temp)) {
            echo "failed to create temporary file...\n";
        }
    }

    function writePost($line, $config) {
        $filename = "config/settings.conf";
        $filename_temp = $filename.".tmp";
        $config_temp = fopen($filename_temp, "r");
        for($i=0;$i<$line - 1;$i++) {
            fgets($config_temp);
        }
        $entry = fgets($config_temp);
        $name = trim(substr($entry,0 , strpos($entry, "=")));
        $value = trim(substr($entry, strpos($entry, "=") + 1));
        if (isset($_POST[$name]) && !empty($_POST[$name])) {
            fwrite($config, $name."=".$_POST[$name]."\n");
        }
        else {
            fwrite($config, $name."=".$value."\n");
        }
        fclose($config_temp);
    }

    if ($_POST['Action'] == 'Default') {
        restoreDefaults();
      } 
    if ($_POST['Action'] == 'Update Settings') {
        storePrevious();    // creates .tmp file of previous values

        $filename = "config/settings.conf";
        $config = fopen($filename, "w+") or die("Unable to open file!"); 
        for($i=1; $i<=15; $i++) {
            writePost($i, $config);
        }
        fclose($config);        
    }
?>
<?php header("Location: index.php"); ?>

</body>
</html>
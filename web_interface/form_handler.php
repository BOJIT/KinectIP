<!DOCTYPE html>
<html>
<body>

<?php
    function WriteSHMEM($Index, $Value) {
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
        //The array to write
        $shared_memory_array = array($Value);
        
        //Convert the array of byte values to a byte string
        $shared_memory_string = call_user_func_array(pack, array_merge(array("C*"), $shared_memory_array));
        shmop_write($shared_memory_id, $shared_memory_string, $Index);			//Shared memory id, string to write, Index to start writing from
                                                                        //Note that a trailing null 0x00 byte is not written, just the byte values / characters, so in this example just 2 bytes are written.
                                                                    
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
        WriteSHMEM(UPDATE_SETTINGS, 255);  
      } 
    if ($_POST['Action'] == 'Update Settings') {
        storePrevious();    // creates .tmp file of previous values
        $filename = "config/settings.conf";
        $config = fopen($filename, "w+") or die("Unable to open file!"); 
        for($i=1; $i<=15; $i++) {
            writePost($i, $config);
        }
        fclose($config);
        WriteSHMEM(UPDATE_SETTINGS, 255);    
    }
?>
<?php header("Location: index.php"); ?>

</body>
</html>
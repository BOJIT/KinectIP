	// Password on Enter
	document.getElementById("password")
	    .addEventListener("keyup", function(event) {
	    event.preventDefault();
	    if (event.keyCode === 13) {
	        document.getElementById("enterPassword").click();
	    }
	});

	// reload telemetry
//	function reload() {
//    document.getElementById('telemetry').src += '';
//} btn.onclick = reload;

	// on page load
	window.onload = changeColor("RGB");
	window.onload = changeColor("DEPTH");
	window.onload = changeColor("INDEX");
	window.onload = changeColor("POINTS");
	window.onload = showIP();

	// Change Button Colour
	function changeColor(id) {
		var w = document.getElementById(id+"0");
		var x = document.getElementById(id);
		var y = document.getElementById(id+"1");
		var z = document.getElementById(id+"2");
		if (x.checked == true){
			y.classList.add("w3-teal");
			y.classList.remove("w3-dark-grey");
			w.disabled = true;
			if (z.className.indexOf("w3-show") == -1) {
				z.className += " w3-show";
			} 
		} 
		else {
			y.classList.add("w3-dark-grey");
			y.classList.remove("w3-teal");
			w.disabled = false;
			if (z.className.indexOf("w3-show") != -1) {
				z.className = z.className.replace(" w3-show", "");
			}
		}
	}
  
  // Tabs
  function openTab(evt, tabName) {
	var i;
	var x = document.getElementsByClassName("tab");
	for (i = 0; i < x.length; i++) {
	  x[i].style.display = "none";
	}
	var activebtn = document.getElementsByClassName("testbtn");
	for (i = 0; i < x.length; i++) {
	  activebtn[i].className = activebtn[i].className.replace(" w3-dark-grey", "");
	}
	document.getElementById(tabName).style.display = "block";
	evt.currentTarget.className += " w3-dark-grey";
  }
  
  var mybtn = document.getElementsByClassName("testbtn")[0];
  mybtn.click();
  
  // Accordions
  function myAccFunc(id) {
		var x = document.getElementById(id);
		if (x.className.indexOf("w3-show") == -1) {
	  	x.className += " w3-show";
		} else { 
	  	x.className = x.className.replace(" w3-show", "");
		}
  }

  // Show IP for Static IP
  function showIP() {
		var StaticIP = document.getElementById("StaticIP");
		var IPBox = document.getElementById("IPBox");
		var IPEntry = document.getElementById("IPEntry");
		if (StaticIP.checked == true){
			IPBox.style.display = "block";
			IPEntry.required = true;
		}
		else{
			IPBox.style.display = "none";
			IPEntry.required = false;
		}
	}
	
	// Password Validation
	function Password() {
		var Password = document.getElementById("password").value;
		var Validate = "BOJIT";
		if(Password == Validate) {
			document.getElementById('id01').style.display='none';
			document.getElementById("password").value='';
			document.getElementById('incorrect').style.display='none';
		}
		else{
			document.getElementById('incorrect').style.display='';
		}
	}



  
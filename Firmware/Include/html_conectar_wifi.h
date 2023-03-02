//Codigo HTML con CSS y Javascript para configurar WIFI estation
char* HTML = R"(

<!doctype html>

<html lang="es">
    
    <head>
        
        <meta charset="utf-8">
        
        <title> Configuracion WIFI </title>    
        
        <meta name="viewport" content="width=device-width, initial-scale=1.0">
        
        <meta name="author" content="Fabian Burgos">
        <meta name="description" content="Configuracion wifi ESP32">
        <meta name="keywords" content="Formulario Configuracion">
        
               
        <style type="text/css">
            body {
					font-family: 'Overpass', sans-serif;
					font-weight: normal;
					font-size: 100%;
					color: #1b262c;
					
					margin: 0;
					background-color: #0f4c75;
				}

				#contenedor {
					display: flex;
					align-items: center;
					justify-content: center;
					
					margin: 0;
					padding: 0;
					min-width: 100vw;
					min-height: 100vh;
					width: 100%;
					height: 100%;
				}

				#central {
					max-width: 320px;
					width: 100%;
				}

				.titulo {
					font-size: 250%;
					color:#bbe1fa;
					text-align: center;
					margin-bottom: 20px;
				}

				#login {
					width: 100%;
					padding: 50px 30px;
					background-color: #3282b8;
					
					-webkit-box-shadow: 0px 0px 5px 5px rgba(0,0,0,0.15);
					-moz-box-shadow: 0px 0px 5px 5px rgba(0,0,0,0.15);
					box-shadow: 0px 0px 5px 5px rgba(0,0,0,0.15);
					
					border-radius: 3px 3px 3px 3px;
					-moz-border-radius: 3px 3px 3px 3px;
					-webkit-border-radius: 3px 3px 3px 3px;
					
					box-sizing: border-box;
				}

				#reiniciando {
					width: 100%;
					padding: 50px 30px;
					background-color: #3282b8;
					
					-webkit-box-shadow: 0px 0px 5px 5px rgba(0,0,0,0.15);
					-moz-box-shadow: 0px 0px 5px 5px rgba(0,0,0,0.15);
					box-shadow: 0px 0px 5px 5px rgba(0,0,0,0.15);
					
					border-radius: 3px 3px 3px 3px;
					-moz-border-radius: 3px 3px 3px 3px;
					-webkit-border-radius: 3px 3px 3px 3px;
					
					box-sizing: border-box;
				}
				#login input {
					font-family: 'Overpass', sans-serif;
					font-size: 110%;
					color: #1b262c;
					
					display: block;
					width: 100%;
					height: 40px;
					
					margin-bottom: 10px;
					padding: 5px 5px 5px 10px;
					
					box-sizing: border-box;
					
					border: none;
					border-radius: 3px 3px 3px 3px;
					-moz-border-radius: 3px 3px 3px 3px;
					-webkit-border-radius: 3px 3px 3px 3px;
				}

				#login input::placeholder {
					font-family: 'Overpass', sans-serif;
					color: #E4E4E4;
				}

				#buttons {
					font-family: 'Overpass', sans-serif;
					font-size: 110%;
					color:#1b262c;
					width: 100%;
					height: 40px;
					border: none;
					
					border-radius: 3px 3px 3px 3px;
					-moz-border-radius: 3px 3px 3px 3px;
					-webkit-border-radius: 3px 3px 3px 3px;
					
					background-color: #bbe1fa;
					
					margin-top: 10px;
				}

				#buttons:hover {
					background-color: #0f4c75;
					color:#bbe1fa;
				}

				.pie-form {
					font-size: 90%;
					text-align: center;    
					margin-top: 15px;
				}

				.pie-form a {
					display: block;
					text-decoration: none;
					color: #bbe1fa;
					margin-bottom: 3px;
				}

				.pie-form a:hover {
					color: #0f4c75;
				}

				.inferior {
					margin-top: 10px;
					font-size: 90%;
					text-align: center;
				}

				.inferior a {
					display: block;
					text-decoration: none;
					color: #bbe1fa;
					margin-bottom: 3px;
				}

				.inferior a:hover {
					color: #3282b8;
				}

        </style>
        
        <script type="text/javascript">
		function getParameterByName(name, url) {
			if (!url) url = window.location.href;
			name = name.replace(/[\[\]]/g, "\\$&");
			var regex = new RegExp("[?&]" + name ),
				results = regex.exec(url);
			if (!results) return null;
			if (!results[2]) return '';
			return decodeURIComponent(results[2].replace(/\+/g, " "));
		}
		 function fun() {  
			var prodId = getParameterByName('Red');
			if (prodId != null){
				var x = document.getElementById("reiniciando");
				x.style.display = "block";
				var y = document.getElementById("login");
				y.style.display = "none";
			}
		   }  
		function enviar(){
			   var x = document.getElementById("reiniciando");
				x.style.display = "block";
				var y = document.getElementById("login");
				y.style.display = "none";
				document.loginform.submit();
		   }
        </script>
        
    </head>
    
    <body onload="fun();">
        
        <div id="contenedor">
            <div id="central">
                <div id="login">
                    <div class="titulo">
                        Bienvenido
                    </div>
                    <form id="loginform" name="loginform" action="" method="get">
                        <input type="text" name="Red" placeholder="Nombre de la red" required>
                        
                        <input type="password" placeholder="Contraseña de la red" name="password" required>
                        
                        <input id="buttons" type=button onclick="enviar();" title="Ingresar" value="Configurar">
                    </form>
                    <div class="pie-form">
                        <a href="javascript:alert('Por favor ingrese la contraseña y nombre de la red a la que desea conectar el dispositivo.');" id="click">¿Necesita ayuda?</a>						
                    </div>
                </div>
				<div id="reiniciando" style="display: none">
                    <div class="titulo">
                        Reiniciando ESP32
                    </div>
					<center>
					<img width="100" height="100" src="data:image/jpeg;base64,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">
					</center>
                </div>
                <div class="inferior">
                    <a href="#">Esp32 Configuracion Wifi - Cargador bateria</a>
                </div>
            </div>
        </div>
            
    </body>
</html>
    )";
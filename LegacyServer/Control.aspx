<%@ Page Language="C#" AutoEventWireup="true" CodeBehind="Control.aspx.cs" Inherits="CollarControl.Control" %>

<!DOCTYPE html>

<html xmlns="http://www.w3.org/1999/xhtml">
<head runat="server">
    <title>Collar Control</title>
    <script src="//ajax.googleapis.com/ajax/libs/jquery/1.11.1/jquery.min.js"></script>
    <style>
        /* http://meyerweb.com/eric/tools/css/reset/
		   v2.0 | 20110126
		   License: none (public domain)
		*/

        html, body, div, span, applet, object, iframe,
        h1, h2, h3, h4, h5, h6, p, blockquote, pre,
        a, abbr, acronym, address, big, cite, code,
        del, dfn, em, img, ins, kbd, q, s, samp,
        small, strike, strong, sub, sup, tt, var,
        b, u, i, center,
        dl, dt, dd, ol, ul, li,
        fieldset, form, label, legend,
        table, caption, tbody, tfoot, thead, tr, th, td,
        article, aside, canvas, details, embed,
        figure, figcaption, footer, header, hgroup,
        menu, nav, output, ruby, section, summary,
        time, mark, audio, video {
            margin: 0;
            padding: 0;
            border: 0;
            font-size: 100%;
            font: inherit;
            vertical-align: baseline;
        }
        /* HTML5 display-role reset for older browsers */
        article, aside, details, figcaption, figure,
        footer, header, hgroup, menu, nav, section {
            display: block;
        }

        body {
            line-height: 1;
        }

        ol, ul {
            list-style: none;
        }

        blockquote, q {
            quotes: none;
        }

            blockquote:before, blockquote:after,
            q:before, q:after {
                content: '';
                content: none;
            }

        table {
            border-collapse: collapse;
            border-spacing: 0;
        }

        /* RESET END */
		
		html, body {
			height: 100%;
		}

        body {
            background-color: #303030;
            color: #eee;
            width: 100%;
            height: 100%;
            font-family: Calibri;
        }

        .wrapper {
            max-width: 600px;
            margin: 0 auto;
        }

        .button {
            width: 100%;
            -webkit-user-select: none; /* Chrome all / Safari all */
            -moz-user-select: none; /* Firefox all */
            -ms-user-select: none; /* IE 10+ */
            user-select: none; /* Likely future */
        }

            .button p {
                line-height: 200px;
                text-align: center;
                font-size: 45px;
            }

        .button-warn {
            background-color: #cc9125;
        }
			
			#btnWarn.heldDown {
                background-color: #8f6312;
			}

        .button-punish {
            background-color: #ea1e1e;
        }
			
			#btnPunish.heldDown {
                background-color: #b91515;
			}
    </style>
</head>
<body onmouseup="clickRelease()" ontouchend="releaseBoth()">
    <form id="form1" runat="server">
        <div class="wrapper">
            <asp:Button ID="btnLogout" runat="server" Text="Log out" OnClick="btnLogout_Click" />
            <br />
            <asp:Literal ID="litUserInfo" runat="server" />
            <br />
            <br />
            <br />
            <div id="btnWarn" class="button button-warn" ontouchstart="touchEventFire('WarningStart')" onmousedown="clickEventFire('WarningStart')" onmouseleave="releaseBoth()">
                <p>Warn</p>
            </div>
            <br />
            <br />
            <br />
            <br />
            <br />
            <br />
            <br />
            <div id="btnPunish" class="button button-punish" ontouchstart="touchEventFire('PunishmentStart')" onmousedown="clickEventFire('PunishmentStart')" onmouseleave="releaseBoth()">
                <p>Punish</p>
            </div>
        </div>
    </form>
</body>
</html>

<script>
	var isTouch = false;

	function touchEventFire(invar) {
		isTouch = true;
		sendData(invar);
	}
	
	function clickEventFire(invar) {
		if (isTouch !== true) {
			sendData(invar);
		}
	}
	
	function clickRelease(e) {
		if (isTouch !== true) {
			releaseBoth();
		}
	}

    function sendData(invar) {
		if (invar === "WarningStart") {
			var warnBtn = document.getElementById("btnWarn");
			warnBtn.classList.add("heldDown");
		}
		if (invar === "PunishmentStart") {
			var punishBtn = document.getElementById("btnPunish");
			punishBtn.classList.add("heldDown");
		}
		if (invar === "AllStop") {
			var warnBtn = document.getElementById("btnWarn");
			warnBtn.classList.remove("heldDown");
			var punishBtn = document.getElementById("btnPunish");
			punishBtn.classList.remove("heldDown");
		}

        var login = JSON.parse(getCookie("User"));

        var obj = {
            username: login.username,
            password: login.password,
            message: invar
        };

        $.ajax({
            //the url to send the data to
            url: "api/Inputs",
            //the data to send to
            data: JSON.stringify(obj),
            //type. for eg: GET, POST
            type: "POST",
            //datatype expected to get in reply form server
            dataType: "json",
            //on success
            success: function (data) {
                //do something after something is recieved from php
            },
            //on error
            error: function () {
                //bad request
            }
        });
    }
    function releaseBoth() {
        sendData('AllStop');
    }
    function getCookie(cname) {
        var name = cname + "=";
        var decodedCookie = decodeURIComponent(document.cookie);
        var ca = decodedCookie.split(';');
        for (var i = 0; i < ca.length; i++) {
            var c = ca[i];
            while (c.charAt(0) == ' ') {
                c = c.substring(1);
            }
            if (c.indexOf(name) == 0) {
                return c.substring(name.length, c.length);
            }
        }
        return "";
    }

    function fnPing() {
        sendData('Ping');
    }
    fnPing();
    setInterval(fnPing, 10 * 1000);
</script>

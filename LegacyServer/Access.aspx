<%@ Page Language="C#" AutoEventWireup="true" CodeBehind="Access.aspx.cs" Inherits="CollarControl.Access" %>

<!DOCTYPE html>

<html xmlns="http://www.w3.org/1999/xhtml">
<head runat="server">
    <title></title>
</head>
<body>
    <form id="form1" runat="server">
        <div>
            <asp:TextBox ID="txtUsername" runat="server" Placeholder="Username" />
            <br />
            <asp:TextBox ID="txtPassword" runat="server" Placeholder="Password" TextMode="Password" />
            <br />
            <asp:Button ID="btnSubmit" runat="server" Text="Log in" OnClick="btnSubmit_Click" />
        </div>
    </form>
</body>
</html>

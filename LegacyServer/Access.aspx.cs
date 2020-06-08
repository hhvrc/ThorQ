using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using System.Web.UI;
using System.Web.UI.WebControls;

namespace ThorQ
{
	public partial class Access : System.Web.UI.Page
	{
		protected void Page_Load(object sender, EventArgs e)
		{
			if (!IsPostBack)
			{
				if (Request.Cookies["User"] != null)
				{
					UserLogin login = JsonConvert.DeserializeObject<UserLogin>(Request.Cookies["User"].Value);

					if (login.IsValid())
					{
						Response.Redirect("~/Control.aspx");
					}
					else
					{
						Response.Cookies["User"].Expires = DateTime.Now.AddDays(-1);
						Response.Redirect("~/");
					}
				}
			}
		}

		protected void btnSubmit_Click(object sender, EventArgs e)
		{
			if (!String.IsNullOrWhiteSpace(txtUsername.Text) && !String.IsNullOrEmpty(txtPassword.Text))
			{
				string username = txtUsername.Text.Trim();
				string password = txtPassword.Text;

				HttpCookie loginCookie = new HttpCookie("User");
				loginCookie.Value = "{\"username\":\"" + username + "\",\"password\":\"" + password + "\"}";
				loginCookie.Expires = DateTime.Now.AddDays(30);
				Response.SetCookie(loginCookie);

				Singleton.AddInput(new UserLogin(username, password), "LoggedIn");

				Response.Redirect("~/Control.aspx");
			}
		}
	}
}
using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using System.Web.UI;
using System.Web.UI.WebControls;

namespace ThorQ
{
	public partial class Control : System.Web.UI.Page
	{
		protected void Page_Load(object sender, EventArgs e)
		{
			if (!IsPostBack)
			{
				if (Request.Cookies["User"] != null)
				{
					UserLogin login = JsonConvert.DeserializeObject<UserLogin>(Request.Cookies["User"].Value);

					if (!login.IsValid())
					{
						Response.Cookies["User"].Expires = DateTime.Now.AddDays(-1);
						Response.Redirect("~/");
					}

					litUserInfo.Text = String.Format("Key: {0}<br/>User: {1} ", login.key, login.username);
				}
				else
				{
					Response.Cookies["User"].Expires = DateTime.Now.AddDays(-1);
					Response.Redirect("~/");
				}
			}
		}

		protected void btnLogout_Click(object sender, EventArgs e)
		{
			try
			{
				if (Request.Cookies["User"] != null)
				{
					UserLogin login = JsonConvert.DeserializeObject<UserLogin>(Request.Cookies["User"].Value);

					lock (Singleton.inputs)
					{
						Singleton.inputs.RemoveAll(i => i.Username.ToLower() == login.username.ToLower());
					}

					Response.Cookies["User"].Expires = DateTime.Now.AddDays(-1);

					Singleton.AddInput(new UserLogin(login.username, login.password), "LoggedOut");
				}

				Response.Redirect("~/");
			}
			catch (Exception)
			{
			}
		}
	}
}
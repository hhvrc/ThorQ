using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Net.Http;
using System.Web.Http;

namespace ThorQ.Controllers
{
	public class InputsController : ApiController
	{
		// GET api/<controller>
		public IEnumerable<Input> Get()
		{
			lock (Singleton.inputs)
			{
				DateTime time = DateTime.UtcNow.AddMinutes(-10);
				Singleton.inputs.RemoveAll(i => DateTime.Parse(i.TimeStamp) < time);

				return Singleton.inputs;
			}
		}

		// GET api/<controller>/5
		public IEnumerable<Input> Get(string id)
		{
			id = id.Replace("i", ":").Replace("d", ".");
			try
			{
				DateTime parsedTime = DateTime.Parse(id);

				lock (Singleton.inputs)
				{
					DateTime time = DateTime.UtcNow.AddMinutes(-10);
					Singleton.inputs.RemoveAll(i => DateTime.Parse(i.TimeStamp) < time);

					return Singleton.inputs.Where(i => DateTime.Parse(i.TimeStamp) > parsedTime);
				}
			}
			catch (Exception)
			{
			}

			return null;
		}

		// POST api/<controller>
		[HttpPost]
		public HttpResponseMessage Post(HttpRequestMessage request)
		{
			string data = request.Content.ReadAsStringAsync().Result;

			if (data != null)
			{
				InputPost input = JsonConvert.DeserializeObject<InputPost>(data);

				Singleton.AddInput(new UserLogin(input.username, input.password), input.message);

				return new HttpResponseMessage(HttpStatusCode.Accepted);
			}

			return new HttpResponseMessage(HttpStatusCode.NotImplemented);
		}

		// PUT api/<controller>/5
		public void Put(int id, [FromBody]string value)
		{
		}

		// DELETE api/<controller>/5
		public void Delete(int id)
		{
		}
	}
}
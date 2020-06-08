using LiteDB;
using System;
using System.Collections.Generic;
using System.Threading;

namespace ThorQ
{
	public class DbCollectionHandler<T>
	{
		private bool _running = false;
		private object _runLock = new object();
		
		private WorkAwaiter workQueue = new WorkAwaiter();

		private Thread _jobDispatcher;
		private ILiteCollection<T> _dbCollection;
		private List<Action<ILiteCollection<T>>> _dbJobQueue = new List<Action<ILiteCollection<T>>>();

		public DbCollectionHandler(LiteDatabase db, String name)
		{
			_dbCollection = db.GetCollection<T>(name);
			_jobDispatcher = new Thread(new ThreadStart(DbJobDispatcher));
			_running = true;
			_jobDispatcher.Start();
		}
		public bool Running
		{
			get
			{
				return _jobDispatcher.IsAlive;
			}
			set
			{
				lock (_runLock)
				{
					if (_running == value)
						return;
					_running = value;
				}

				if (value == true && !_jobDispatcher.IsAlive)
				{
					_jobDispatcher.Start();
				}
				else if (value == false && _jobDispatcher.IsAlive)
				{
					_jobDispatcher.Join();
				}
			}
		}

		void DbJobDispatcher()
		{
			Action<ILiteCollection<T>> action = null;
			while (Running)
			{
				while (!workQueue.HasWork(100))
					if (!Running)
						return;

				action = null;
				lock (_dbJobQueue)
				{
					if (_dbJobQueue.Count != 0)
					{
						action = _dbJobQueue[0];
						_dbJobQueue.RemoveAt(0);
					}
				}
				try
				{
					action?.Invoke(_dbCollection);
				}
				catch (Exception ex)
				{
					Console.WriteLine($"Database job threw an exception: {ex.Message}");
				}
				finally
				{
					workQueue.WorkDone();
				}
			}
		}

		public void AddJob(Action<ILiteCollection<T>> job)
		{
			lock (_dbJobQueue)
				_dbJobQueue.Add(job);
			workQueue.AddWork();
		}
	}
}

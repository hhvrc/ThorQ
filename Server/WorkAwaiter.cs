using System.Threading;

namespace ThorQ
{
	public class WorkAwaiter
	{
		private int count = 0;
		private ManualResetEvent _resetEvent = new ManualResetEvent(false);

		public bool WorkDone()
		{
			return _resetEvent.Reset();
		}

		public void AddWork()
		{
			if (Interlocked.Increment(ref count) == 1)
				_resetEvent.Set();
		}

		public bool HasWork(int milliseconds)
		{
			if (count != 0 || _resetEvent.WaitOne(milliseconds))
			{
				Interlocked.Decrement(ref count);
				return true;
			}
			return false;
		}
	}
}

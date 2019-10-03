using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CollarControl
{
    public static class ToolBox
    {
        public static T[] CompileArrays<T>(T[] array, T[] appendArray, params T[][] additional)
        {
            long resultLength = array.Length + appendArray.Length;
            long offsetSize = resultLength;

            foreach (T[] arr in additional)
            {
                resultLength += arr.Length;
            }

            T[] result = new T[resultLength];

            array.CopyTo(result, 0);
            appendArray.CopyTo(result, array.Length);

            foreach (T[] arr in additional)
            {
                arr.CopyTo(result, offsetSize);
                offsetSize += arr.Length;
            }

            return result;
        }

        public static T[] SubArray<T>(this T[] data, int index, int length)
        {
            T[] result = new T[length];
            Array.Copy(data, index, result, 0, length);
            return result;
        }

        public static byte[] ToNetworkLayer(int value)
        {
            byte[] bytes = BitConverter.GetBytes(value);
            if (BitConverter.IsLittleEndian)
                Array.Reverse(bytes);
            return bytes;
        }

        public static int ToHostLayer(byte[] bytes)
        {
            if (BitConverter.IsLittleEndian)
                Array.Reverse(bytes);
            return BitConverter.ToInt32(bytes, 0);
        }
    }
}

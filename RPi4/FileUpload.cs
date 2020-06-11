using System;
using System.IO;
using System.Net;
using System.Text;
using System.Collections.Generic;

using static System.Console;

namespace CDLFileUpload
{
    class FileUpload
    {
        static string fileName = "";
        static readonly string server = "ftp://chlwlsgur96.ipdisk.co.kr/HDD1/Data/Project/CDL/";
        static readonly string serverId = "ftpUser";
        static readonly string serverPw = "1598462";


        static void Main(string[] args)
        {
            Write("Initializing...");

            try
            {
                fileName = args[0];
            }
            catch (Exception ex)
            {
                WriteLine("Fail");
                ReadLine();

                return;
            }

            Write($"Uploading File ({fileName})...");

            if (UploadFile(fileName))
            {
                WriteLine("Ok");
            }
            else
            {
                WriteLine("Fail");
            }
        }

        static bool UploadFile(string localPath)
        {
            string fileName = Path.GetFileName(localPath);
            string serverPath = $"{server}{fileName}";

            try
            {
                using (var wc = new WebClient())
                {
                    wc.Credentials = new NetworkCredential(serverId, serverPw);
                    wc.UploadFile(serverPath, localPath);
                }
            }
            catch (Exception ex)
            {
                return false;
            }

            return true;
        }
    }
}
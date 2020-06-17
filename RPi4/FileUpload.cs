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
        static List<string> filePaths;
        static readonly string server = "ftp://chlwlsgur96.ipdisk.co.kr/HDD1/Data/Project/CDL/";
        static readonly string serverId = "ftpUser";
        static readonly string serverPw = "1598462";


        static void Main(string[] args)
        {
            Write("Initializing...");

            try
            {
                filePaths = new List<string>();

                for (int i = 0; i < args.Length; ++i)
                {
                    filePaths.Add(args[i]);
                }

                WriteLine($"Total file count : {filePaths.Count}");
            }
            catch (Exception)
            {
                WriteLine("Fail");
                ReadLine();

                return;
            }

            using (var wc = new WebClient())
            {
                int count = 1;
                int total = filePaths.Count;

                wc.Credentials = new NetworkCredential(serverId, serverPw);

                foreach (var fName in filePaths)
                {
                    Write($"Uploading File ({count} / {total})...");

                    if (UploadFile(wc, fName))
                    {
                        WriteLine("Ok");
                    }
                    else
                    {
                        WriteLine("Fail");
                    }
                }
            }
        }

        static bool UploadFile(WebClient wc, string localPath)
        {
            string fileName = Path.GetFileName(localPath);
            string serverPath = $"{server}{fileName}";

            try
            {
                wc.UploadFile(serverPath, localPath);
            }
            catch (Exception ex)
            {
                WriteLine(ex.ToString());

                return false;
            }

            return true;
        }
    }
}
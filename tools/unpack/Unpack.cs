/// <summary>
/// Unpack -- a completely non-object oriented utility...
/// </summary>

using System;
using System.Text;
using System.IO;

class Unpack
{
    static int IDPAKHEADER = (('K' << 24) + ('C' << 16) + ('A' << 8) + 'P');
    static int DIRECTORY_LENGTH = 64;
    static int DIRECTORY_NAME_LENGTH = DIRECTORY_LENGTH - 8;

    static int IntSwap(int i)
    {
        int a, b, c, d;

        if (BitConverter.IsLittleEndian) {
            return i;
        }

        a = i & 255;
        b = (i >> 8) & 255;
        c = (i >> 16) & 255;
        d = (i >> 24) & 255;

        return (a << 24) + (b << 16) + (c << 8) + d;
    }

    static bool PatternMatch(string pattern, string s)
    {
        int index;
        int remaining;

        if (pattern.Equals(s)) {
            return true;
        }

        // fairly lame single wildcard matching
        index = pattern.IndexOf('*');

        if (index == -1) {
            return false;
        }

        if (String.Compare(pattern, 0, s, 0, index) != 0) {
            return false;
        }

        index += 1; // skip the *
        remaining = pattern.Length - index;

        if (s.Length < remaining) {
            return false;
        }

        if (String.Compare(pattern, index, s, s.Length - remaining,
            remaining) != 0) {
            return false;
        }

        return true;
    }

    static void Usage()
    {
        System.Console.WriteLine("Usage: unpack <packfile> <match> <basedir>");
        System.Console.WriteLine("   or: unpack -list <packfile>");
        System.Console.WriteLine("<match> may contain a single * wildcard");
        System.Environment.Exit(1);
    }

    static void Main(string[] args)
    {
        String pakName;
        String pattern;

        if (args.Length == 2) {
            if (!args[0].Equals("-list")) {
                Usage();
            }

            pakName = args[1];
            pattern = null;

        } else if (args.Length == 3) {

            pakName = args[0];
            pattern = args[1];

        } else {

            pakName = null;
            pattern = null;
            Usage();
        }

        try {

            Stream readLump;
            BinaryReader directory;

            // one stream to read the directory
            directory = new BinaryReader(new FileStream(pakName,
                FileMode.Open, FileAccess.Read));

            // another to read lumps
            readLump = new FileStream(pakName, FileMode.Open,
                FileAccess.Read);

            // read the header
            var ident = IntSwap(directory.ReadInt32());
            var dirofs = IntSwap(directory.ReadInt32());
            var dirlen = IntSwap(directory.ReadInt32());

            if (ident != IDPAKHEADER) {

                System.Console.WriteLine(pakName + " is not a pakfile.");
                System.Environment.Exit(1);
            }

            // read the directory
            directory.ReadBytes(dirofs - 12); // skipBytes
            var numlumps = dirlen / DIRECTORY_LENGTH;

            System.Console.WriteLine(numlumps + " lumps in " + pakName);

            for (int i = 0; i < numlumps; i++) {

                var filename = directory.ReadBytes(DIRECTORY_NAME_LENGTH);
                var filepos = IntSwap(directory.ReadInt32());
                var filelen = IntSwap(directory.ReadInt32());

                String filenameStr = Encoding.Default.GetString(filename);

                // chop to the first 0 byte
                filenameStr = filenameStr.Substring(0,
                    filenameStr.IndexOf('\0'));

                if (pattern == null) {

                    // listing mode
                    System.Console.WriteLine(filenameStr + " : " +
                        filelen + "bytes");

                } else if (PatternMatch(pattern, filenameStr)) {

                    Stream writeFile;
                    BinaryWriter writeLump;

                    byte[] buffer = new byte[filelen];
                    String pathName;
                    int index;

                    System.Console.WriteLine("Unpaking " + filenameStr
                        + " " + filelen + " bytes");

                    // load the lump
                    readLump.Seek(filepos, SeekOrigin.Begin);
                    readLump.Read(buffer, 0, filelen);

                    // quake uses forward slashes, but java requires
                    // they only be the host's seperator, which
                    // varies from win to unix
                    StringBuilder fixedStr = new StringBuilder(args[2] +
                        Path.DirectorySeparatorChar + filenameStr);

                    // replace quake path seperator with system path seperator
                    fixedStr.Replace('/', Path.DirectorySeparatorChar);
                    pathName = fixedStr.ToString();

                    index = pathName.LastIndexOf(Path.DirectorySeparatorChar);

                    if (index != -1) {

                        String path = pathName.Substring(0, index);
                        Directory.CreateDirectory(path);
                    }

                    writeFile = File.Create(pathName);
                    writeLump = new BinaryWriter(writeFile);
                    writeLump.Write(buffer);
                    writeLump.Close();
                }
            }

            readLump.Close();
            directory.Close();

        } catch (IOException e) {
            
            System.Console.WriteLine(e.ToString());
            System.Environment.Exit(2);
        }
    }
}

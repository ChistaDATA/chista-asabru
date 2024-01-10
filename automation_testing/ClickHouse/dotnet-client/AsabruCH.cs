using System.Data;
using ClickHouse.Ado;

class AsabruCH
{
    private readonly string CONFIG_FILE = "../ch_config.yaml";
    private readonly string SQL_FILE = "../ch_sql.yaml";

    private string _chHost = string.Empty;
    private string _chUser = string.Empty;
    private string _chPassword = string.Empty;
    private string _datasetUrl = string.Empty;
    private string _datasetFileName = string.Empty;
    private string _datasetCsvName = string.Empty;
    private string _databaseName = string.Empty;
    private string _tableName = string.Empty;
    private int _port = 0;
    private string _caCert = string.Empty;
    private string _clientConfig = string.Empty;
    private bool _ssl = false;

    private Dictionary<string, Dictionary<string, string>> _sqlStatements = new Dictionary<string, Dictionary<string, string>>();

    public AsabruCH()
    {
        // Load configuration from YAML file
        LoadConfiguration();
        // Load SQL statements from YAML file
        LoadSqlStatements();
    }

    private void LoadConfiguration()
    {
        Console.WriteLine("Started : LoadConfiguration");
        // Load configuration from YAML file
        var config = new YamlDotNet.Serialization.Deserializer().Deserialize<Dictionary<string, string>>(
            File.ReadAllText(CONFIG_FILE));

        _chHost = config["host"];
        _chUser = config["username"];
        _chPassword = config["password"];
        _datasetUrl = config["dataset_url"];
        _datasetFileName = config["dataset_file_name"];
        _datasetCsvName = config["dataset_csv_name"];
        _databaseName = config["database"];
        _tableName = config["table"];
        _port = int.Parse(config["port"]);
        _caCert = config["ca_cert"];
        _clientConfig = config["client_config"];
        _ssl = bool.Parse(config["ssl"].ToLower());

        if (_ssl)
        {
            Console.WriteLine("Using SSL/TLS");
        }
        Console.WriteLine("Finished : LoadConfiguration");
    }

    private void LoadSqlStatements()
    {
        _sqlStatements = new YamlDotNet.Serialization.Deserializer().Deserialize<Dictionary<string, Dictionary<string, string>>>(
            File.ReadAllText(SQL_FILE));
    }



    private readonly string CommonConnectionString = "Compress=True;CheckCompressedHash=False;SocketTimeout=60000;";
    private ClickHouseConnection CreateConnection()
    {
        var sslConnectionString = _ssl
            ? $"SslMode=VerifyFull;SslCa={_caCert};"
            : string.Empty;

        var connectionString = $"{CommonConnectionString}Host={_chHost};Port={_port};User={_chUser};Password={_chPassword};{sslConnectionString}";

        Console.WriteLine("Connection String : " + connectionString);
        return new ClickHouseConnection(connectionString);
    }

    public void BenchmarkPerformanceOld()
    {
        var loopStart = DateTime.Now;
        var loopTimes = new List<double>();
        var iters = 10;

        Console.WriteLine("Starting the benchmark");
        using (var connection = CreateConnection())
        {
            connection.Open();
            for (var x = 0; x < iters; x++)
            {
                Console.WriteLine($"Iteration {x + 1}");

                var iterStart = DateTime.Now;

                foreach (var queryKey in _sqlStatements["read"].Keys)
                {
                    var query = _sqlStatements["read"][queryKey];
                    connection.CreateCommand(query).ExecuteNonQuery();
                    Console.WriteLine(query);
                }

                var iterEnd = (DateTime.Now - iterStart).TotalSeconds;

                loopTimes.Add(iterEnd);
            }
            connection.Close();
        }

        Console.WriteLine($"Average Time taken: {loopTimes.Average()} seconds");
        Console.WriteLine($"Median Time taken: {loopTimes[loopTimes.Count / 2]} seconds");
    }

    public void BenchmarkPerformance()
    {
        var loopStart = DateTime.Now;
        var loopTimes = new List<double>();
        var iters = 10;

        Console.WriteLine("Starting the benchmark");

        using (var connection = CreateConnection())
        {
            connection.Open();

            for (var x = 0; x < iters; x++)
            {
                Console.WriteLine($"Iteration {x + 1}");

                var iterStart = DateTime.Now;

                foreach (var queryKey in _sqlStatements["read"].Keys)
                {
                    var query = _sqlStatements["read"][queryKey];

                    using (var cmd = new ClickHouseCommand(connection, query))
                    {
                        cmd.ExecuteNonQuery();
                    }

                    Console.WriteLine($"Query: {query}, Time: {(DateTime.Now - iterStart).TotalSeconds} seconds");
                }

                var iterEnd = (DateTime.Now - iterStart).TotalSeconds;

                loopTimes.Add(iterEnd);
            }

            connection.Close();
        }

        Console.WriteLine($"Average Time taken: {loopTimes.Average()} seconds");
        Console.WriteLine($"Median Time taken: {loopTimes[loopTimes.Count / 2]} seconds");
    }

}

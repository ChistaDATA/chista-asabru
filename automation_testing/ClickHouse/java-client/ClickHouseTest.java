import java.io.File;
import java.io.IOException;
import java.sql.Connection;
import java.sql.ResultSet;
import java.sql.Statement;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Properties;

import com.clickhouse.jdbc.ClickHouseDataSource;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.fasterxml.jackson.dataformat.yaml.YAMLFactory;

public class ClickHouseTest {
    static String jdbcUrl;
    static Configuration configuration;
    static QueryConfig query_config;

    static List<String> sql_statements;

    public static void main(String[] args) throws IOException {
        ObjectMapper mapper = new ObjectMapper(new YAMLFactory());

        // Read the YAML file
        configuration = mapper.readValue(new File("../ch_config.yaml"), Configuration.class);
        query_config = mapper.readValue(new File("../ch_sql.yaml"), QueryConfig.class);

        // JDBC URL, username, and password of ClickHouse server
        jdbcUrl = "jdbc:clickhouse://" + configuration.host + ":" + configuration.port;

        sql_statements = query_config.read.values().stream().toList();
        benchmark_performance();
    }

    public static void benchmark_performance() {
        System.out.println("Starting the benchmark");
        long startTime = System.nanoTime();

        // Your code to benchmark goes here
        // ...
        ArrayList<Double> loop_times = new ArrayList<>();

        int iterations = 10;
        int queries = sql_statements.size();

        for (int i = 0; i < iterations; i++) {
            // JDBC variables for opening, closing, and managing a connection
            Connection connection = null;
            Statement statement = null;
            ResultSet resultSet = null;

            System.out.println("Iteration " + (i + 1));

            Properties properties = new Properties();
            properties.setProperty("ssl", "false");
            properties.setProperty("sslmode", "none"); // NONE to trust all servers; STRICT for trusted only
//                properties.setProperty("sslrootcert", configuration.ca_cert);

            try {
                ClickHouseDataSource dataSource = new ClickHouseDataSource(jdbcUrl, properties);

                for (String stmt: sql_statements) {
                    // Execute a query
                    connection = dataSource.getConnection(configuration.username, configuration.password);
                    statement = connection.createStatement();
                    statement.executeQuery(stmt);
                    System.out.println(stmt);
                    connection.close();
                }
            } catch (Exception e) {
                e.printStackTrace();
            } finally {
                // Close resources in the reverse order of their creation
                try {
                    if (resultSet != null) resultSet.close();
                    if (statement != null) statement.close();
                    if (connection != null) connection.close();
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
            long endTime = System.nanoTime();
            long elapsedTime = endTime - startTime;
            loop_times.add((double) elapsedTime / 1_000_000_000);
        }

        System.out.println("Average Time taken : " + findMean(loop_times));
        System.out.println("Median Time taken : " + findMedian(loop_times));

    }

    static double findMean(ArrayList<Double> numArray) {
        return numArray.stream()
                .mapToDouble(d -> d)
                .average()
                .orElse(0.0);
    }

    static double findMedian(ArrayList<Double> numArray) {
        Collections.sort(numArray);
        double median;
        if (numArray.size() % 2 == 0)
            median = (numArray.get(numArray.size() / 2) + numArray.get(numArray.size() / 2 - 1))/2;
        else
            median = numArray.get(numArray.size() / 2);
        return median;
    }
}
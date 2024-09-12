/*
# Filename: GstoreConnector.java
# Author: suxunbin
# Last Modified: 2021-07-21 17:00
# Description: http api for java
*/
package com.bdkg.test;

import java.io.*;
import java.net.*;
import java.lang.*;
import java.net.URLEncoder;
import java.net.URLDecoder;
import java.io.UnsupportedEncodingException;
import java.util.List;
import java.util.Map;

public class GstoreConnector {

    public static final String defaultServerIP = "127.0.0.1";
    public static final int defaultServerPort = 9000;

    private String serverIP;
    private int serverPort;
    private String Url;
    private String username;
    private String password;

    public GstoreConnector(String _ip, int _port, String _http_type, String _user, String _passwd) {
        if (_ip.equals("localhost")) {
            this.serverIP = GstoreConnector.defaultServerIP;
        } else {
            this.serverIP = _ip;
        }
        this.serverPort = _port;
        this.Url = "http://" + this.serverIP + ":" + this.serverPort + "/";
        if ("grpc".equals(_http_type)) {
            this.Url = this.Url + "grpc/api";
        }
        this.username = _user;
        this.password = _passwd;
    }

    public String sendGet(String strUrl) {
        StringBuffer result = new StringBuffer();
        BufferedReader in = null;

        try {
            strUrl = URLEncoder.encode(strUrl, "UTF-8");
        } catch (UnsupportedEncodingException ex) {
            throw new RuntimeException("Broken VM does not support UTF-8");
        }

        try {
            strUrl = this.Url + strUrl;
            URL realUrl = new URL(strUrl);

            // open the connection with the URL
            URLConnection connection = realUrl.openConnection();

            // set request properties
            connection.setRequestProperty("accept", "*/*");
            connection.setRequestProperty("connection", "Keep-Alive");
            connection.setRequestProperty("user-agent", "Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1;SV1)");

            // create the real connection
            connection.connect();

            // get all response header fields
            Map<String, List<String>> map = connection.getHeaderFields();

            // define BufferedReader to read the response of the URL
            in = new BufferedReader(new InputStreamReader(connection.getInputStream(), "utf-8"));
            String line;
            while ((line = in.readLine()) != null) {
                result.append(line + "\n");
            }
        } catch (Exception e) {
            System.out.println("error in get request: " + e);
            e.printStackTrace();
        }

        // use finally to close the input stream
        finally {
            try {
                if (in != null) {
                    in.close();
                }
            } catch (Exception e2) {
                e2.printStackTrace();
            }
        }
        return result.toString();
    }

    public String sendPost(String strPost) {
        String strUrl = "";
        PrintWriter out = null;
        StringBuffer result = new StringBuffer();
        BufferedReader in = null;

        try {
            strUrl = URLEncoder.encode(strUrl, "UTF-8");
        } catch (UnsupportedEncodingException ex) {
            throw new RuntimeException("Broken VM does not support UTF-8");
        }

        try {
            strUrl = this.Url;
            URL realUrl = new URL(strUrl);

            // open the connection with the URL
            URLConnection connection = realUrl.openConnection();

            // set request properties
            connection.setRequestProperty("accept", "*/*");
            connection.setRequestProperty("connection", "Keep-Alive");
            connection.setRequestProperty("user-agent", "Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1;SV1)");

            connection.setDoOutput(true);
            connection.setDoInput(true);
            out = new PrintWriter(connection.getOutputStream());
            out.print(strPost);
            out.flush();

            // get all response header fields
            Map<String, List<String>> map = connection.getHeaderFields();

            // define BufferedReader to read the response of the URL
            in = new BufferedReader(new InputStreamReader(connection.getInputStream(), "utf-8"));
            String line;
            while ((line = in.readLine()) != null) {
                result.append(line + "\n");
            }
        } catch (Exception e) {
            System.out.println("error in post request: " + e);
            e.printStackTrace();
        }

        // use finally to close the input stream
        finally {
            try {
                if (out != null) {
                    out.close();
                }
                if (in != null) {
                    in.close();
                }
            } catch (IOException ex) {
                ex.printStackTrace();
            }
        }
        return result.toString();
    }

    public void sendGet(String strUrl, String filename) {
        BufferedReader in = null;
        if (filename == null)
            return;

        FileWriter fw = null;
        try {
            fw = new FileWriter(filename);
        } catch (IOException e) {
            System.out.println("can not open " + filename + "!");
        }

        try {
            strUrl = URLEncoder.encode(strUrl, "UTF-8");
        } catch (UnsupportedEncodingException ex) {
            throw new RuntimeException("Broken VM does not support UTF-8");
        }

        try {
            strUrl = this.Url + strUrl;
            URL realUrl = new URL(strUrl);

            // open the connection with the URL
            URLConnection connection = realUrl.openConnection();

            // set request properties
            connection.setRequestProperty("accept", "*/*");
            connection.setRequestProperty("connection", "Keep-Alive");
            connection.setRequestProperty("user-agent", "Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1;SV1)");

            // create the real connection
            connection.connect();

            // get all response header fields
            Map<String, List<String>> map = connection.getHeaderFields();

            // define BufferedReader to read the response of the URL
            in = new BufferedReader(new InputStreamReader(connection.getInputStream(), "utf-8"));
            char chars[] = new char[2048];
            int b;
            while ((b = in.read(chars, 0, 2048)) != -1) {
                if (fw != null)
                    fw.write(chars);
                chars = new char[2048];
            }
        } catch (Exception e) {
            System.out.println("error in get request: " + e);
            e.printStackTrace();
        }

        // use finally to close the input stream
        finally {
            try {
                if (in != null) {
                    in.close();
                }
                if (fw != null) {
                    fw.close();
                }
            } catch (Exception e2) {
                e2.printStackTrace();
            }
        }
        return;
    }

    public void sendPost(String strPost, String filename) {
        String strUrl = "";
        PrintWriter out = null;
        BufferedReader in = null;

        if (filename == null)
            return;

        FileWriter fw = null;
        try {
            fw = new FileWriter(filename);
        } catch (IOException e) {
            System.out.println("can not open " + filename + "!");
        }

        try {
            strUrl = URLEncoder.encode(strUrl, "UTF-8");
        } catch (UnsupportedEncodingException ex) {
            throw new RuntimeException("Broken VM does not support UTF-8");
        }

        try {
            strUrl = this.Url;
            URL realUrl = new URL(strUrl);

            // open the connection with the URL
            URLConnection connection = realUrl.openConnection();

            // set request properties
            connection.setRequestProperty("accept", "*/*");
            connection.setRequestProperty("connection", "Keep-Alive");
            connection.setRequestProperty("user-agent", "Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1;SV1)");

            connection.setDoOutput(true);
            connection.setDoInput(true);
            out = new PrintWriter(connection.getOutputStream());
            out.print(strPost);
            out.flush();

            // get all response header fields
            Map<String, List<String>> map = connection.getHeaderFields();

            // define BufferedReader to read the response of the URL
            in = new BufferedReader(new InputStreamReader(connection.getInputStream(), "utf-8"));
            char chars[] = new char[2048];
            int b;
            while ((b = in.read(chars, 0, 2048)) != -1) {
                if (fw != null)
                    fw.write(chars);
                chars = new char[2048];
            }
        } catch (Exception e) {
            System.out.println("error in post request: " + e);
            e.printStackTrace();
        }

        // use finally to close the input stream
        finally {
            try {
                if (out != null) {
                    out.close();
                }
                if (in != null) {
                    in.close();
                }
                if (fw != null) {
                    fw.close();
                }
            } catch (IOException ex) {
                ex.printStackTrace();
            }
        }
        return;
    }


    public String check(String request_type) {
        String res = "";
        if (request_type.equals("GET")) {
            String strUrl = "?operation=check";
            res = this.sendGet(strUrl);
        } else if (request_type.equals("POST")) {
            String strPost = "{\"operation\": \"check\"}";
            res = this.sendPost(strPost);
        }
        return res;
    }

    public String check() {
        String res = this.check("GET");
        return res;
    }


    public String login(String request_type) {
        String res = "";
        if (request_type.equals("GET")) {
            String strUrl = "?operation=login&username=" + this.username + "&password=" + this.password;
            res = this.sendGet(strUrl);
        } else if (request_type.equals("POST")) {
            String strPost = "{\"operation\": \"login\", \"username\": \"" + this.username + "\", \"password\": \"" + this.password + "\"}";
            res = this.sendPost(strPost);
        }
        return res;
    }

    public String login() {
        String res = this.login("GET");
        return res;
    }

    public String testConnect(String request_type) {
        String res = "";
        if (request_type.equals("GET")) {
            String strUrl = "?operation=testConnect&username=" + this.username + "&password=" + this.password;
            res = this.sendGet(strUrl);
        } else if (request_type.equals("POST")) {
            String strPost = "{\"operation\": \"testConnect\", \"username\": \"" + this.username + "\", \"password\": \"" + this.password + "\"}";
            res = this.sendPost(strPost);
        }
        return res;
    }

    public String testConnect() {
        String res = this.testConnect("GET");
        return res;
    }


    public String getCoreVersion(String request_type) {
        String res = "";
        if (request_type.equals("GET")) {
            String strUrl = "?operation=getCoreVersion&username=" + this.username + "&password=" + this.password;
            res = this.sendGet(strUrl);
        } else if (request_type.equals("POST")) {
            String strPost = "{\"operation\": \"getCoreVersion\", \"username\": \"" + this.username + "\", \"password\": \"" + this.password + "\"}";
            res = this.sendPost(strPost);
        }
        return res;
    }

    public String getCoreVersion() {
        String res = this.getCoreVersion("GET");
        return res;
    }


    public String ipmanage(String type, String ip_type, String ips, String request_type) {
        String res = "";
        if (request_type.equals("GET")) {
            String strUrl = "?operation=ipmanage&type=" + type + "&ip_type=" + ip_type + "&ips=" + ips + "&username=" + this.username + "&password=" + this.password;
            res = this.sendGet(strUrl);
        } else if (request_type.equals("POST")) {
            String strPost = "{\"operation\": \"ipmanage\",\"type\":\"" + type + "\",\"ip_type\":\"" + ip_type + "\", \"ips\":\"" + ips + "\", \"username\": \"" + this.username + "\", \"password\": \"" + this.password + "\"}";
            res = this.sendPost(strPost);
        }
        return res;
    }

    public String ipmanage(String type, String ip_type, String ips) {
        String res = this.ipmanage(type, ip_type, ips, "GET");
        return res;
    }

    public String upload(String filepath, String request_type) {
        String res = "";
        if (request_type.equals("GET")) {
            String strUrl = "?operation=upload&filepath=" + filepath + "&username=" + this.username + "&password=" + this.password;
            res = this.sendGet(strUrl);
        } else if (request_type.equals("POST")) {
            String strPost = "{\"operation\": \"upload\",\"filepath\":\"" + filepath + "\", \"username\": \"" + this.username + "\", \"password\": \"" + this.password + "\"}";
            res = this.sendPost(strPost);
        }
        return res;
    }

    public String upload(String filepath) {
        String res = this.upload(filepath, "GET");
        return res;
    }


    public String download(String filepath, String request_type) {
        String res = "";
        if (request_type.equals("GET")) {
            String strUrl = "?operation=download&filepath=" + filepath + "&username=" + this.username + "&password=" + this.password;
            res = this.sendGet(strUrl);
        } else if (request_type.equals("POST")) {
            String strPost = "{\"operation\": \"download\",\"filepath\":\"" + filepath + "\", \"username\": \"" + this.username + "\", \"password\": \"" + this.password + "\"}";
            res = this.sendPost(strPost);
        }
        return res;
    }

    public String download(String filepath) {
        String res = this.download(filepath, "GET");
        return res;
    }


    public String stat(String request_type) {
        String res = "";
        if (request_type.equals("GET")) {
            String strUrl = "?operation=stat&username=" + this.username + "&password=" + this.password;
            res = this.sendGet(strUrl);
        } else if (request_type.equals("POST")) {
            String strPost = "{\"operation\": \"stat\", \"username\": \"" + this.username + "\", \"password\": \"" + this.password + "\"}";
            res = this.sendPost(strPost);
        }
        return res;
    }

    public String stat() {
        String res = this.stat("GET");
        return res;
    }

    public String shutdown(String request_type) {
        String res = "";
        if (request_type.equals("GET")) {
            String strUrl = "?operation=shutdown&username=" + this.username + "&password=" + this.password;
            res = this.sendGet(strUrl);
        } else if (request_type.equals("POST")) {
            String strPost = "{\"operation\": \"shutdown\", \"username\": \"" + this.username + "\", \"password\": \"" + this.password + "\"}";
            res = this.sendPost(strPost);
        }
        return res;
    }

    public String shutdown() {
        String res = this.shutdown("GET");
        return res;
    }


    public String show(String request_type) {
        String res = "";
        if (request_type.equals("GET")) {
            String strUrl = "?operation=show&username=" + this.username + "&password=" + this.password;
            res = this.sendGet(strUrl);
        } else if (request_type.equals("POST")) {
            String strPost = "{\"operation\": \"show\", \"username\": \"" + this.username + "\", \"password\": \"" + this.password + "\"}";
            res = this.sendPost(strPost);
        }
        return res;
    }

    public String show() {
        String res = this.show("GET");
        return res;
    }


    public String load(String db_name, String csr, String request_type) {
        String res = "";
        if (csr == null || "".equals(csr)) {
            csr = "0";
        }
        if (request_type.equals("GET")) {
            String strUrl = "?operation=load&db_name=" + db_name + "&csr=" + csr + "&username=" + this.username + "&password=" + this.password;
            res = this.sendGet(strUrl);
        } else if (request_type.equals("POST")) {
            String strPost = "{\"operation\": \"load\", \"db_name\": \"" + db_name + "\", \"csr\": \"" + csr + "\", \"username\": \"" + this.username + "\", \"password\": \"" + this.password + "\"}";
            res = this.sendPost(strPost);
        }
        return res;
    }

    public String load(String db_name, String csr) {
        String res = this.load(db_name, csr, "GET");
        return res;
    }

    public String unload(String db_name, String request_type) {
        String res = "";
        if (request_type.equals("GET")) {
            String strUrl = "?operation=unload&db_name=" + db_name + "&username=" + this.username + "&password=" + this.password;
            res = this.sendGet(strUrl);
        } else if (request_type.equals("POST")) {
            String strPost = "{\"operation\": \"unload\", \"db_name\": \"" + db_name + "\", \"username\": \"" + this.username + "\", \"password\": \"" + this.password + "\"}";
            res = this.sendPost(strPost);
        }
        return res;
    }

    public String unload(String db_name) {
        String res = this.unload(db_name, "GET");
        return res;
    }


    public String monitor(String db_name, String request_type) {
        String res = "";
        if (request_type.equals("GET")) {
            String strUrl = "?operation=monitor&db_name=" + db_name + "&username=" + this.username + "&password=" + this.password;
            res = this.sendGet(strUrl);
        } else if (request_type.equals("POST")) {
            String strPost = "{\"operation\": \"monitor\", \"db_name\": \"" + db_name + "\", \"username\": \"" + this.username + "\", \"password\": \"" + this.password + "\"}";
            res = this.sendPost(strPost);
        }
        return res;
    }

    public String monitor(String db_name) {
        String res = this.monitor(db_name, "GET");
        return res;
    }

    public String build(String db_name, String db_path, String async, String callback, String request_type) {
        String res = "";
        if (request_type.equals("GET")) {
            String strUrl = "?operation=build&db_name=" + db_name + "&db_path=" + db_path + "&async=" + async + "&callback=" + callback + "&username=" + this.username + "&password=" + this.password;
            res = this.sendGet(strUrl);
        } else if (request_type.equals("POST")) {
            String strPost = "{\"operation\": \"build\", \"db_name\": \"" + db_name + "\", \"db_path\": \"" + db_path + "\", \"async\": \"" + async + "\", \"callback\": \"" + callback + "\", \"username\": \"" + this.username + "\", \"password\": \"" + this.password + "\"}";
            res = this.sendPost(strPost);
        }
        return res;
    }

    public String build(String db_name, String db_path, String async, String callback) {
        String res = this.build(db_name, db_path, async, callback, "GET");
        return res;
    }


    public String drop(String db_name, boolean is_backup, String request_type) {
        String res = "";
        if (request_type.equals("GET")) {
            String strUrl;
            if (is_backup) {
                strUrl = "?operation=drop&db_name=" + db_name + "&username=" + this.username + "&password=" + this.password + "&is_backup=true";
            } else {
                strUrl = "?operation=drop&db_name=" + db_name + "&username=" + this.username + "&password=" + this.password + "&is_backup=false";
            }
            res = this.sendGet(strUrl);
        } else if (request_type.equals("POST")) {
            String strPost;
            if (is_backup) {
                strPost = "{\"operation\": \"drop\", \"db_name\": \"" + db_name + "\", \"username\": \"" + this.username + "\", \"password\": \"" + this.password + "\", \"is_backup\": \"true\"}";
            } else {
                strPost = "{\"operation\": \"drop\", \"db_name\": \"" + db_name + "\", \"username\": \"" + this.username + "\", \"password\": \"" + this.password + "\", \"is_backup\": \"false\"}";
            }
            res = this.sendPost(strPost);
        }
        return res;
    }

    public String drop(String db_name, boolean is_backup) {
        String res = this.drop(db_name, is_backup, "GET");
        return res;
    }

    public String backup(String db_name, String backup_path, String async, String callback, String request_type) {
        String res = "";
        if (request_type.equals("GET")) {
            String strUrl = "?operation=backup&db_name=" + db_name + "&backup_path=" + backup_path + "&async=" + async + "&callback=" + callback + "&username=" + this.username + "&password=" + this.password;
            res = this.sendGet(strUrl);
        } else if (request_type.equals("POST")) {
            String strPost = "{\"operation\": \"backup\", \"db_name\": \"" + db_name + "\", \"backup_path\": \"" + backup_path + "\", \"async\": \"" + async + "\", \"callback\": \"" + callback + "\", \"username\": \"" + this.username + "\", \"password\": \"" + this.password + "\"}";
            res = this.sendPost(strPost);
        }
        return res;
    }

    public String backup(String db_name, String backup_path, String async, String callback) {
        String res = this.backup(db_name, backup_path, async, callback, "GET");
        return res;
    }


    public String backuppath(String db_name, String request_type) {
        String res = "";
        if (request_type.equals("GET")) {
            String strUrl = "?operation=backuppath&db_name=" + db_name + "&username=" + this.username + "&password=" + this.password;
            res = this.sendGet(strUrl);
        } else if (request_type.equals("POST")) {
            String strPost = "{\"operation\": \"backuppath\", \"db_name\": \"" + db_name + "\", \"username\": \"" + this.username + "\", \"password\": \"" + this.password + "\"}";
            res = this.sendPost(strPost);
        }
        return res;
    }

    public String backuppath(String db_name) {
        String res = this.backuppath(db_name, "GET");
        return res;
    }


    public String restore(String db_name, String backup_path, String async, String callback, String request_type) {
        String res = "";
        if (request_type.equals("GET")) {
            String strUrl = "?operation=restore&async=" + async + "&callback=" + callback + "&username=" + this.username + "&password=" + this.password + "&db_name=" + db_name + "&backup_path=" + backup_path;
            res = this.sendGet(strUrl);
        } else if (request_type.equals("POST")) {
            String strPost = "{\"operation\": \"restore\",\"async\": \"" + async + "\",\"callback\": \"" + callback + "\", \"username\": \"" + this.username + "\", \"password\": \"" + this.password + "\", \"db_name\": \"" + db_name + "\", \"backup_path\": \"" + backup_path + "\"}";
            res = this.sendPost(strPost);
        }
        return res;
    }

    public String restore(String db_name, String backup_path, String async, String callback) {
        String res = this.restore(db_name, backup_path, async, callback, "GET");
        return res;
    }


    public String query(String db_name, String format, String sparql, String request_type) {
        String res = "";
        if (request_type.equals("GET")) {
            String strUrl = "?operation=query&username=" + this.username + "&password=" + this.password + "&db_name=" + db_name + "&format=" + format + "&sparql=" + sparql;
            res = this.sendGet(strUrl);
        } else if (request_type.equals("POST")) {
            String strPost = "{\"operation\": \"query\", \"username\": \"" + this.username + "\", \"password\": \"" + this.password + "\", \"db_name\": \"" + db_name + "\", \"format\": \"" + format + "\", \"sparql\": \"" + sparql + "\"}";
            res = this.sendPost(strPost);
        }
        return res;
    }

    public String query(String db_name, String format, String sparql) {
        String res = this.query(db_name, format, sparql, "GET");
        return res;
    }

    public String exportDB(String db_name, String db_path, String request_type) {
        String res = "";
        if (request_type.equals("GET")) {
            String strUrl = "?operation=export&username=" + this.username + "&password=" + this.password + "&db_name=" + db_name + "&db_path=" + db_path;
            res = this.sendGet(strUrl);
        } else if (request_type.equals("POST")) {
            String strPost = "{\"operation\": \"export\", \"username\": \"" + this.username + "\", \"password\": \"" + this.password + "\", \"db_name\": \"" + db_name + "\", \"db_path\": \"" + db_path + "\"}";
            res = this.sendPost(strPost);
        }
        return res;
    }

    public String exportDB(String db_name, String db_path) {
        String res = this.exportDB(db_name, db_path, "GET");
        return res;
    }


    public String batchInsert(String db_name, String file, String dir, String async, String callback, String request_type) {
        String res = "";
        if (request_type.equals("GET")) {
            String strUrl = "?operation=batchInsert&dir=" + dir + "&async=" + async + "&callback=" + callback + "&username=" + this.username + "&password=" + this.password + "&db_name=" + db_name + "&file=" + file;
            res = this.sendGet(strUrl);
        } else if (request_type.equals("POST")) {
            String strPost = "{\"operation\": \"batchInsert\",\"dir\": \"" + dir + "\",\"async\": \"" + async + "\",\"callback\": \"" + callback + "\", \"username\": \"" + this.username + "\", \"password\": \"" + this.password + "\", \"db_name\": \"" + db_name + "\", \"file\": \"" + file + "\"}";
            res = this.sendPost(strPost);
        }
        return res;
    }

    public String batchInsert(String db_name, String file, String dir, String async, String callback) {
        String res = this.batchInsert(db_name, file, dir, async, callback, "GET");
        return res;
    }


    public String batchRemove(String db_name, String file, String async, String callback, String request_type) {
        String res = "";
        if (request_type.equals("GET")) {
            String strUrl = "?operation=batchRemove&async=" + async + "&callback=" + callback + "&username=" + this.username + "&password=" + this.password + "&db_name=" + db_name + "&file=" + file;
            res = this.sendGet(strUrl);
        } else if (request_type.equals("POST")) {
            String strPost = "{\"operation\": \"batchRemove\",\"async\": \"" + async + "\",\"callback\": \"" + callback + "\", \"username\": \"" + this.username + "\", \"password\": \"" + this.password + "\", \"db_name\": \"" + db_name + "\", \"file\": \"" + file + "\"}";
            res = this.sendPost(strPost);
        }
        return res;
    }

    public String batchRemove(String db_name, String file, String async, String callback) {
        String res = this.batchRemove(db_name, file, async, callback, "GET");
        return res;
    }


    public String rename(String db_name, String new_name, String request_type) {
        String res = "";
        if (request_type.equals("GET")) {
            String strUrl = "?operation=rename&username=" + this.username + "&password=" + this.password + "&db_name=" + db_name + "&new_name=" + new_name;
            res = this.sendGet(strUrl);
        } else if (request_type.equals("POST")) {
            String strPost = "{\"operation\": \"rename\", \"username\": \"" + this.username + "\", \"password\": \"" + this.password + "\", \"db_name\": \"" + db_name + "\", \"new_name\": \"" + new_name + "\"}";
            res = this.sendPost(strPost);
        }
        return res;
    }

    public String rename(String db_name, String new_name) {
        String res = this.rename(db_name, new_name, "GET");
        return res;
    }


    public String checkOperationState(String opt_id, String request_type) {
        String res = "";
        if (request_type.equals("GET")) {
            String strUrl = "?operation=checkOperationState&opt_id=" + opt_id + "&username=" + this.username + "&password=" + this.password;
            res = this.sendGet(strUrl);
        } else if (request_type.equals("POST")) {
            String strPost = "{\"operation\": \"checkOperationState\", \"opt_id\": \"" + opt_id + "\", \"username\": \"" + this.username + "\", \"password\": \"" + this.password + "\"}";
            res = this.sendPost(strPost);
        }
        return res;
    }

    public String checkOperationState(String opt_id) {
        String res = this.checkOperationState(opt_id, "GET");
        return res;
    }


    public String begin(String db_name, String isolevel, String request_type) {
        String res = "";
        if (request_type.equals("GET")) {
            String strUrl = "?operation=begin&username=" + this.username + "&password=" + this.password + "&db_name=" + db_name + "&isolevel=" + isolevel;
            res = this.sendGet(strUrl);
        } else if (request_type.equals("POST")) {
            String strPost = "{\"operation\": \"begin\", \"username\": \"" + this.username + "\", \"password\": \"" + this.password + "\", \"db_name\": \"" + db_name + "\", \"isolevel\": \"" + isolevel + "\"}";
            res = this.sendPost(strPost);
        }
        return res;
    }

    public String begin(String db_name, String isolevel) {
        String res = this.begin(db_name, isolevel, "GET");
        return res;
    }


    public String tquery(String db_name, String tid, String sparql, String request_type) {
        String res = "";
        if (request_type.equals("GET")) {
            String strUrl = "?operation=tquery&username=" + this.username + "&password=" + this.password + "&db_name=" + db_name + "&tid=" + tid + "&sparql=" + sparql;
            res = this.sendGet(strUrl);
        } else if (request_type.equals("POST")) {
            String strPost = "{\"operation\": \"tquery\", \"username\": \"" + this.username + "\", \"password\": \"" + this.password + "\", \"db_name\": \"" + db_name + "\", \"tid\": \"" + tid + "\", \"sparql\": \"" + sparql + "\"}";
            res = this.sendPost(strPost);
        }
        return res;
    }

    public String tquery(String db_name, String tid, String sparql) {
        String res = this.tquery(db_name, tid, sparql, "GET");
        return res;
    }


    public String commit(String db_name, String tid, String request_type) {
        String res = "";
        if (request_type.equals("GET")) {
            String strUrl = "?operation=commit&username=" + this.username + "&password=" + this.password + "&db_name=" + db_name + "&tid=" + tid;
            res = this.sendGet(strUrl);
        } else if (request_type.equals("POST")) {
            String strPost = "{\"operation\": \"commit\", \"username\": \"" + this.username + "\", \"password\": \"" + this.password + "\", \"db_name\": \"" + db_name + "\", \"tid\": \"" + tid + "\"}";
            res = this.sendPost(strPost);
        }
        return res;
    }

    public String commit(String db_name, String tid) {
        String res = this.commit(db_name, tid, "GET");
        return res;
    }

    public String rollback(String db_name, String tid, String request_type) {
        String res = "";
        if (request_type.equals("GET")) {
            String strUrl = "?operation=rollback&username=" + this.username + "&password=" + this.password + "&db_name=" + db_name + "&tid=" + tid;
            res = this.sendGet(strUrl);
        } else if (request_type.equals("POST")) {
            String strPost = "{\"operation\": \"rollback\", \"username\": \"" + this.username + "\", \"password\": \"" + this.password + "\", \"db_name\": \"" + db_name + "\", \"tid\": \"" + tid + "\"}";
            res = this.sendPost(strPost);
        }
        return res;
    }

    public String rollback(String db_name, String tid) {
        String res = this.rollback(db_name, tid, "GET");
        return res;
    }


    public String checkpoint(String db_name, String request_type) {
        String res = "";
        if (request_type.equals("GET")) {
            String strUrl = "?operation=checkpoint&username=" + this.username + "&password=" + this.password + "&db_name=" + db_name;
            res = this.sendGet(strUrl);
        } else if (request_type.equals("POST")) {
            String strPost = "{\"operation\": \"checkpoint\", \"username\": \"" + this.username + "\", \"password\": \"" + this.password + "\", \"db_name\": \"" + db_name + "\"}";
            res = this.sendPost(strPost);
        }
        return res;
    }

    public String checkpoint(String db_name) {
        String res = this.checkpoint(db_name, "GET");
        return res;
    }


    public String showuser(String request_type) {
        String res = "";
        if (request_type.equals("GET")) {
            String strUrl = "?operation=showuser&username=" + this.username + "&password=" + this.password;
            res = this.sendGet(strUrl);
        } else if (request_type.equals("POST")) {
            String strPost = "{\"operation\": \"showuser\", \"username\": \"" + this.username + "\", \"password\": \"" + this.password + "\"}";
            res = this.sendPost(strPost);
        }
        return res;
    }

    public String showuser() {
        String res = this.showuser("GET");
        return res;
    }

    public String usermanage(String type, String op_username, String op_password, String request_type) {
        String res = "";
        if (request_type.equals("GET")) {
            String strUrl = "?operation=usermanage&type=" + type + "&username=" + this.username + "&password=" + this.password + "&op_username=" + op_username + "&op_password=" + op_password;
            res = this.sendGet(strUrl);
        } else if (request_type.equals("POST")) {
            String strPost = "{\"operation\": \"usermanage\", \"type\": \"" + type + "\", \"username\": \"" + this.username + "\", \"password\": \"" + this.password + "\", \"op_username\": \"" + op_username + "\", \"op_password\": \"" + op_password + "\"}";
            res = this.sendPost(strPost);
        }
        return res;
    }

    public String usermanage(String type, String op_username, String op_password) {
        String res = this.usermanage(type, op_username, op_password, "GET");
        return res;
    }


    public String userprivilegemanage(String type, String op_username, String privileges, String db_name, String request_type) {
        String res = "";
        if (request_type.equals("GET")) {
            String strUrl = "?operation=userprivilegemanage&type=" + type + "&username=" + this.username + "&password=" + this.password + "&op_username=" + op_username + "&privileges=" + privileges + "&db_name=" + db_name;
            res = this.sendGet(strUrl);
        } else if (request_type.equals("POST")) {
            String strPost = "{\"operation\": \"userprivilegemanage\", \"type\": \"" + type + "\", \"username\": \"" + this.username + "\", \"password\": \"" + this.password + "\", \"op_username\": \"" + op_username + "\", \"privileges\": \"" + privileges + "\", \"db_name\": \"" + db_name + "\"}";
            res = this.sendPost(strPost);
        }
        return res;
    }

    public String userprivilegemanage(String type, String op_username, String privileges, String db_name) {
        String res = this.userprivilegemanage(type, op_username, privileges, db_name, "GET");
        return res;
    }


    public String userpassword(String re_username, String re_password, String op_password, String db_name, String request_type) {
        String res = "";
        if (request_type.equals("GET")) {
            String strUrl = "?operation=userpassword&re_username=" + re_username + "&username=" + this.username + "&password=" + this.password + "&re_password=" + re_password + "&op_password=" + op_password + "&db_name=" + db_name;
            res = this.sendGet(strUrl);
        } else if (request_type.equals("POST")) {
            String strPost = "{\"operation\": \"userpassword\", \"re_username\": \"" + re_username + "\", \"username\": \"" + this.username + "\", \"password\": \"" + this.password + "\", \"re_password\": \"" + re_password + "\", \"op_password\": \"" + op_password + "\", \"db_name\": \"" + db_name + "\"}";
            res = this.sendPost(strPost);
        }
        return res;
    }

    public String userpassword(String re_username, String re_password, String op_password, String db_name) {
        String res = this.userpassword(re_username, re_password, op_password, db_name, "GET");
        return res;
    }


    public String funquery(String funInfo, String request_type) {
        String res = "";
        if (request_type.equals("GET")) {
            String strUrl = "?operation=funquery&username=" + this.username + "&password=" + this.password + "&funInfo=" + funInfo;
            res = this.sendGet(strUrl);
        } else if (request_type.equals("POST")) {
            String strPost = "{\"operation\": \"funquery\", \"username\": \"" + this.username + "\", \"password\": \"" + this.password + "\", \"funInfo\": \"" + funInfo + "\"}";
            res = this.sendPost(strPost);
        }
        return res;
    }

    public String funquery(String funInfo) {
        String res = this.funquery(funInfo, "GET");
        return res;
    }

    public String funcudb(String type, String funInfo, String request_type) {
        String res = "";
        if (request_type.equals("GET")) {
            String strUrl = "?operation=funcudb&type=" + type + "&username=" + this.username + "&password=" + this.password + "&funInfo=" + funInfo;
            res = this.sendGet(strUrl);
        } else if (request_type.equals("POST")) {
            String strPost = "{\"operation\": \"funcudb\",\"type\": \"" + type + "\",  \"username\": \"" + this.username + "\", \"password\": \"" + this.password + "\", \"funInfo\": \"" + funInfo + "\"}";
            res = this.sendPost(strPost);
        }
        return res;
    }

    public String funcudb(String type, String funInfo) {
        String res = this.funcudb(type, funInfo, "GET");
        return res;
    }


    public String funreview(String funInfo, String request_type) {
        String res = "";
        if (request_type.equals("GET")) {
            String strUrl = "?operation=funreview&username=" + this.username + "&password=" + this.password + "&funInfo=" + funInfo;
            res = this.sendGet(strUrl);
        } else if (request_type.equals("POST")) {
            String strPost = "{\"operation\": \"funreview\", \"username\": \"" + this.username + "\", \"password\": \"" + this.password + "\", \"funInfo\": \"" + funInfo + "\"}";
            res = this.sendPost(strPost);
        }
        return res;
    }

    public String funreview(String funInfo) {
        String res = this.funreview(funInfo, "GET");
        return res;
    }

    public String txnlog(String request_type) {
        String res = "";
        if (request_type.equals("GET")) {
            String strUrl = "?operation=txnlog&username=" + this.username + "&password=" + this.password;
            res = this.sendGet(strUrl);
        } else if (request_type.equals("POST")) {
            String strPost = "{\"operation\": \"txnlog\", \"username\": \"" + this.username + "\", \"password\": \"" + this.password + "\"}";
            res = this.sendPost(strPost);
        }
        return res;
    }

    public String txnlog() {
        String res = this.txnlog("GET");
        return res;
    }


    public String querylogdate(String request_type) {
        String res = "";
        if (request_type.equals("GET")) {
            String strUrl = "?operation=querylogdate&username=" + this.username + "&password=" + this.password;
            res = this.sendGet(strUrl);
        } else if (request_type.equals("POST")) {
            String strPost = "{\"operation\": \"querylogdate\", \"username\": \"" + this.username + "\", \"password\": \"" + this.password + "\"}";
            res = this.sendPost(strPost);
        }
        return res;
    }

    public String querylogdate() {
        String res = this.querylogdate("GET");
        return res;
    }


    public String querylog(String date, String pageNo, String pageSize, String db_name, String request_type) {
        String res = "";
        if (request_type.equals("GET")) {
            String strUrl = "?operation=querylog&date=" + date + "&username=" + this.username + "&password=" + this.password + "&pageNo=" + pageNo + "&pageSize=" + pageSize + "&db_name=" + db_name;
            res = this.sendGet(strUrl);
        } else if (request_type.equals("POST")) {
            String strPost = "{\"operation\": \"querylog\", \"date\": \"" + date + "\", \"username\": \"" + this.username + "\", \"password\": \"" + this.password + "\", \"pageNo\": \"" + pageNo + "\", \"pageSize\": \"" + pageSize + "\", \"db_name\": \"" + db_name + "\"}";
            res = this.sendPost(strPost);
        }
        return res;
    }

    public String querylog(String date, String pageNo, String pageSize, String db_name) {
        String res = this.querylog(date, pageNo, pageSize, db_name, "GET");
        return res;
    }

    public String accesslogdate(String request_type) {
        String res = "";
        if (request_type.equals("GET")) {
            String strUrl = "?operation=accesslogdate&username=" + this.username + "&password=" + this.password;
            res = this.sendGet(strUrl);
        } else if (request_type.equals("POST")) {
            String strPost = "{\"operation\": \"accesslogdate\", \"username\": \"" + this.username + "\", \"password\": \"" + this.password + "\"}";
            res = this.sendPost(strPost);
        }
        return res;
    }

    public String accesslogdate() {
        String res = this.accesslogdate("GET");
        return res;
    }


    public String accesslog(String date, String pageNo, String pageSize, String request_type) {
        String res = "";
        if (request_type.equals("GET")) {
            String strUrl = "?operation=accesslog&date=" + date + "&username=" + this.username + "&password=" + this.password + "&pageNo=" + pageNo + "&pageSize=" + pageSize;
            res = this.sendGet(strUrl);
        } else if (request_type.equals("POST")) {
            String strPost = "{\"operation\": \"accesslog\", \"date\": \"" + date + "\", \"username\": \"" + this.username + "\", \"password\": \"" + this.password + "\", \"pageNo\": \"" + pageNo + "\", \"pageSize\": \"" + pageSize + "\"}";
            res = this.sendPost(strPost);
        }
        return res;
    }

    public String accesslog(String date, String pageNo, String pageSize) {
        String res = this.accesslog(date, pageNo, pageSize, "GET");
        return res;
    }

    public String addReason(String type, String db_name, String ruleinfo, String request_type) {
        String res = "";
        if (request_type.equals("GET")) {
            String strUrl = "?operation=reasonManage&type=" + type + "&username=" + this.username + "&password=" + this.password + "&db_name=" + db_name + "&ruleinfo=" + ruleinfo;
            res = this.sendGet(strUrl);
        } else if (request_type.equals("POST")) {
            String strPost = "{\"operation\": \"reasonManage\", \"type\": \"" + type + "\", \"username\": \"" + this.username + "\", \"password\": \"" + this.password + "\", \"db_name\": \"" + db_name + "\", \"ruleinfo\": \"" + ruleinfo + "\"}";
            res = this.sendPost(strPost);
        }
        return res;
    }

    public String addReason(String type, String db_name, String ruleinfo) {
        String res = this.addReason(type, db_name, ruleinfo, "GET");
        return res;
    }

    public String reasonManage(String type, String db_name, String rulename, String request_type) {
        String res = "";
        if (request_type.equals("GET")) {
            String strUrl = "?operation=reasonManage&type=" + type + "&username=" + this.username + "&password=" + this.password + "&db_name=" + db_name + "&rulename=" + rulename;
            res = this.sendGet(strUrl);
        } else if (request_type.equals("POST")) {
            String strPost = "{\"operation\": \"reasonManage\", \"type\": \"" + type + "\", \"username\": \"" + this.username + "\", \"password\": \"" + this.password + "\", \"db_name\": \"" + db_name + "\", \"rulename\": \"" + rulename + "\"}";
            res = this.sendPost(strPost);
        }
        return res;
    }

    public String reasonManage(String type, String db_name, String rulename) {
        String res = this.reasonManage(type, db_name, rulename, "GET");
        return res;
    }


    private static byte[] packageMsgData(String _msg) {
        //byte[] data_context = _msg.getBytes();
        byte[] data_context = null;
        try {
            data_context = _msg.getBytes("utf-8");
        } catch (UnsupportedEncodingException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
            System.err.println("utf-8 charset is unsupported.");
            data_context = _msg.getBytes();
        }
        int context_len = data_context.length + 1; // 1 byte for '\0' at the end of the context.
        int data_len = context_len + 4; // 4 byte for one int(data_len at the data's head).
        byte[] data = new byte[data_len];

        // padding head(context_len).
        byte[] head = GstoreConnector.intToByte4(context_len);
        for (int i = 0; i < 4; i++) {
            data[i] = head[i];
        }

        // padding context.
        for (int i = 0; i < data_context.length; i++) {
            data[i + 4] = data_context[i];
        }
        // in C, there should be '\0' as the terminator at the end of a char array. so we need add '\0' at the end of sending message.
        data[data_len - 1] = 0;

        return data;
    }

    private static byte[] intToByte4(int _x) // with Little Endian format.
    {
        byte[] ret = new byte[4];
        ret[0] = (byte) (_x);
        ret[1] = (byte) (_x >>> 8);
        ret[2] = (byte) (_x >>> 16);
        ret[3] = (byte) (_x >>> 24);

        return ret;
    }

    private static int byte4ToInt(byte[] _b) // with Little Endian format.
    {
        int byte0 = _b[0] & 0xFF, byte1 = _b[1] & 0xFF, byte2 = _b[2] & 0xFF, byte3 = _b[3] & 0xFF;
        int ret = (byte0) | (byte1 << 8) | (byte2 << 16) | (byte3 << 24);

        return ret;
    }

}


<?php
    //1.建立连接  
     $connect=mysqli_connect('0.0.0.0','root','123','test','8083');
    //2.定义sql语句
        $sql='select * from students';
        //mysqli_query($connect,'set names utf8');
    //3.发送SQL语句
        $result=mysqli_query($connect,$sql);
        $arr=array();//定义空数组
        while($row =mysqli_fetch_array($result)){
            var_dump($row);
                //array_push(要存入的数组，要存的值)
            //array_push($arr,$row);
        }
        var_dump($result);
    //4.关闭连接
       mysqli_close($connect);
 ?>
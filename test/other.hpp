#pragma once

std::string_view server_key = R"(
-----BEGIN PRIVATE KEY-----
MIIEvAIBADANBgkqhkiG9w0BAQEFAASCBKYwggSiAgEAAoIBAQC5T/0SZgJpwOBC
AerAGM9lVhXyuEFslCVEwGJeCQXXx9Nv7ChJ2gBJ9omtTz6JKENcZQ/y2HAvYlgW
/You5Ug0rnTh7CBCNosdcZK+JouhpxmvZ7if50QuUfeOm07Y8lnQ4nKTSSssR2vM
UQYGyxwQkGx0bp/BkR08Lqs+2mWqxgoU/pz4mLThblJ7FJ57zDC6btxMSmXKF2fE
Z8hcM/TVPsNkTe4+l5fgL1vlQvpB/ye2k0TN7XitkDHOvZkVgAEOCHPq7IMACa0b
oKWJqnjpaybLZVr5r7OUj/3J4OmJilR68zSCsYQZhfgB0B2lZybYmMfBECUHYW70
2xdphWKzAgMBAAECggEAQSTH27Rli8SkcT/YRNBT9BU3oPw575ACq/EPg2wblDfo
cbNslE2hBV84FedCHhg72dqjKSUCBv5E5kwZzBOI9xAPAa6uxmgDLoYS5mqJcnna
EyncosW7V6cz5ftd0/EaNs6sJW+20BmJYSloe3HiO0ZyAdozqem4A8SKKY2/6LCb
vDe90fvEGOVD8KieXkAekwk5ngLLaGE/ZqpEDbfNK8iZeFIEByVUYTv+Xzni3jU0
yEWmqB5GTzmazjS2zxAKdWYDS3jJ596q0ZPLEVOPRzWRDQqSKiMIQvR8c0OWAlkC
zTTmu2hmEiCtX65Cwcu3B893UsD3ou59erdpuc1DyQKBgQD+HEVScjfPdNX2vRWk
1k50vUGvQFY+2Fub2ShWCXWNtz/Z/TxpMrJlhqyjXUq5IGodxWh/Mp89exkZOmje
EjH/tr+g+H2JzptJVNr7jaA+qW5k94HM5xmzsynnld/7IRd4aLst1frE6v/S9YR2
Gi4hTXEypdtfkWbjBkunJdRV2wKBgQC6sMCw8LClGf8x1bD7HVrKIIaDbtsHd9Ls
N2l8W32J6EoMER5ZmlstIcZrIn/Iz6o31YyidmX9OItaQKGAhGMSssS9tPeqwL1e
3iwivyJzPsAFw6XIMNSFemrG6APUFEmT2O31+Fsw6qfdEpxxmc6tgoEMQvdzrzbf
rfI5L6x6CQKBgHyfRswpYf+nUCwDDbNauv17t9QIHIQTvdHQDX0shG9tGK6DCHab
O2yZQN+ZqETkhcZ2A0qme/w9+9AomjeW+thrOs3qMD8+iV/Mctz910ID/DUeQZVJ
HYzI6FtAYKmb0BPoYSwYjjS2/6HpcP4u1sLUZRiuyMi02Z2rf7zIy7sTAoGAfafg
Wnf2fXntAU7yi2KrrCaOQ0LHG9Sz3BhD4Nb3vW3jARI4mljBi7QTCCM1XSUhTeln
eiY06WY4rWuZCJaS1JAMyh9XH9AQ5hymGlEJ0AXpWzffisoLEThfTDf387Qf/eph
6E/cmSUoDS2srO24ZaLUVztvsKa7SXyb74++CHkCgYAaVdjYPezCA90v/wZQwa/n
+jA/RJG4ranEH+059YwSEu7WYbb8mDVDryL7r0jCPsexOxD6ILYTro9CJOv7aD0F
n2nhZD1aEhHfFNJLl701/N72AfO8X+QgJS0gHKAVnoZX3wjIOtTWBwZ2lO1dbaMJ
HmS8bGWCKdxjNhwIUJSMbg==
-----END PRIVATE KEY-----

)";

std::string_view server_crt = R"(
-----BEGIN CERTIFICATE-----
MIIDfjCCAmagAwIBAgIUQ5m7PzT+YtgC8SPFQf/zUi52t2UwDQYJKoZIhvcNAQEL
BQAwRTELMAkGA1UEBhMCQ04xIzAhBgNVBAoMGlBlb3BsZSdzIFJlcHVibGljIG9m
IENoaW5hMREwDwYDVQQDDAhDaGluYSBDQTAeFw0yMzA3MTIxMDQ1MTdaFw0zMzA3
MDkxMDQ1MTdaMEAxETAPBgNVBAoMCE15RG9tYWluMREwDwYDVQQLDAhNeURvbWFp
bjEYMBYGA1UEAwwPd3d3LmV4YW1wbGUuY29tMIIBIjANBgkqhkiG9w0BAQEFAAOC
AQ8AMIIBCgKCAQEAuU/9EmYCacDgQgHqwBjPZVYV8rhBbJQlRMBiXgkF18fTb+wo
SdoASfaJrU8+iShDXGUP8thwL2JYFv2KLuVINK504ewgQjaLHXGSviaLoacZr2e4
n+dELlH3jptO2PJZ0OJyk0krLEdrzFEGBsscEJBsdG6fwZEdPC6rPtplqsYKFP6c
+Ji04W5SexSee8wwum7cTEplyhdnxGfIXDP01T7DZE3uPpeX4C9b5UL6Qf8ntpNE
ze14rZAxzr2ZFYABDghz6uyDAAmtG6Cliap46Wsmy2Va+a+zlI/9yeDpiYpUevM0
grGEGYX4AdAdpWcm2JjHwRAlB2Fu9NsXaYViswIDAQABo2swaTAnBgNVHREEIDAe
ggtleGFtcGxlLmNvbYIPd3d3LmV4YW1wbGUuY29tMB0GA1UdDgQWBBQbByI8jgpT
WMXM/mEKBrzQdVGI/DAfBgNVHSMEGDAWgBSz8f0p/cebANK6TyVMNithgRCiQDAN
BgkqhkiG9w0BAQsFAAOCAQEAlb+8Q5Fd9t+LsdHbQQWvfpJ1py4LWz/Jb8pUoEIr
DdL1VcJvm8alsJOenNXFrtoBR4pYXwHHtoOfn9SIVPPAHPaiJKNVR8p1ZR4WJbGJ
gdHwtyLnHqsO5j9tnh0fOhmts14vDnn087LlpG43qLoHFs7em4cmpfDCkeaLFApZ
UrAPSsWqKe2uCIu9V3UnEHDgAFhDK+71u2l7pwNbnL0ja2RNxjllSPZxkxLPPkb7
cASs7kLmzPvaGCVHwlUSmpUS2btNCmctVcRLfjYe2EH/mh6cKPKuDojizQKEujof
+Tt2MAySn88GhNAEu6h0DAECk7HzIl1XZsHR+lBwa8Bz7g==
-----END CERTIFICATE-----

)";

std::string_view ca_crt = R"(
-----BEGIN CERTIFICATE-----
MIIDazCCAlOgAwIBAgIUBLBpI5lp5wxnyVdRZSWL1Ol8OKIwDQYJKoZIhvcNAQEL
BQAwRTELMAkGA1UEBhMCQ04xIzAhBgNVBAoMGlBlb3BsZSdzIFJlcHVibGljIG9m
IENoaW5hMREwDwYDVQQDDAhDaGluYSBDQTAeFw0yMzA3MTIxMDQyNTVaFw0zMzA3
MDkxMDQyNTVaMEUxCzAJBgNVBAYTAkNOMSMwIQYDVQQKDBpQZW9wbGUncyBSZXB1
YmxpYyBvZiBDaGluYTERMA8GA1UEAwwIQ2hpbmEgQ0EwggEiMA0GCSqGSIb3DQEB
AQUAA4IBDwAwggEKAoIBAQDGbwtTpsHF4FoGTIjX0m+4gBkakGZmb5y6oGu7nSLC
VNy1487Niosv7GBAPgygyXZ2YFR2gRWzF4l1kOGi389SaHnbELu8sjueGkw20lGQ
042xi6UT1R6phbDzZgeWD9bXhZEUqqy/kdlieV3ruregA3icXOv4XRgKP1al4fn/
fc9dmbNPUKdpXUcJuv8vs9ycJV1hulSGMMa2vGZDbCB8514qux5HmsX3UB3GzxNH
UOHPIT+WphZ92aIstmZ43RaTJGzOwJ7qc/rbaZZaZhi9tOB16s7eForFFhfziobe
aRo73Jxymw0xDGWUEalmGhWiMZxajcG4ANa5z9ISO7DFAgMBAAGjUzBRMB0GA1Ud
DgQWBBSz8f0p/cebANK6TyVMNithgRCiQDAfBgNVHSMEGDAWgBSz8f0p/cebANK6
TyVMNithgRCiQDAPBgNVHRMBAf8EBTADAQH/MA0GCSqGSIb3DQEBCwUAA4IBAQA0
OKiCI85z9DtgKVV0zcMkVKZUKAttbRaNUEV3IP7wlR48WcNAyrkTWC8qQNLdZMev
Dqb0vICDnKGAgiCEg4FDn4thmkjN00MRtjk7AH9XN7UfmRrxpZnjsRa54kmFrQG1
K2CrCQ8CHcqSmS4lvGSHC3HugYEUNwReHogi6C+LZHdEkxxu5UihcZxu/cB+KH2S
qauLtCxNh25QgxwZE6tV1R6jUXUYSw4X7YJSlHxpL6OC/C6nKJ1iVS+lpmdaKkN/
cEpyuubIw5Lz7jqranVjbZIrLPRQAufmI7osbGv2vhbxCbOxE6GBQ7kwBf1sMuPU
nKE92PSBFQ6YfGH3dKjx
-----END CERTIFICATE-----

)";

std::string_view dh = R"(
-----BEGIN DH PARAMETERS-----
MIIBDAKCAQEA3YpHhhKyBIK3CKb1ak1RFkGgS9t+OeFWvus5jnbMBYY3OZtFnv23
TBMecQ3cIbn10LPUXa2dkvzpZTSvQKJdU/z4iCK7/eOHOeCOYporN+ItmEbuZoEI
jzLkUJF3QzEEF641+RBQLv2Qf6zIjkvOyKCQZKQpdrDdGvKFgQgbUZSkVS9rIvED
dle9s6OmBIMzL1CzMay7wYj8G/x09a4olvepEbTPAAzXbemOT26DJ+a/PflGb5y4
tz+zA7RqlKdgV46tteu0KAf8DRsubrlr3q+HQb6pT4+ZMJKOjeqjLQrVGFtxn1R7
phMGAq/2ItQLYLI+R+nnAvERB1WRFzjmpwIBAgICAOE=
-----END DH PARAMETERS-----

)";


std::string_view client_key = R"(
-----BEGIN PRIVATE KEY-----
MIIEvgIBADANBgkqhkiG9w0BAQEFAASCBKgwggSkAgEAAoIBAQDQuSX3WoBAIMnd
8SimgyRqFbIJD1ZnG4amqZSgD53eZt6iq6VrYKlpktL1kNEiPov6Fd7d9si+X6WG
YjRaDP6zK/5lAv5B8y0ewQRIT5FqhZOBh/qP5/Z0uxlGLXV5HOIqJLDo32n02sOa
/u6qiTR22Ban9cXj+OyBw05Q6aX6viDUfrrlI/vBGd0XREXZvqyIZJ6sHqA8wvw6
2+UE/NMvib+aqbY3rypb8p4crc2l1pxvTCJ1xpj+Bo/fwPbp+UO56wJSxfpkHxRv
QYKNXEH4ccvVaDTm8Mc1sIT9VcIeP99rCxW9lxepx/CEXkPQRDUo3OxVVOwkicR0
DvpTjyPFAgMBAAECggEAQCYHQyRvckYhhLLPmY2IrYoJc7thBMNy+Nn5sc/a5zWa
i0hRyNSmfKoBaDOf8vRiNPRv0Kw+OjRPPSQFjv+bcBm/T/BBkKdaYK9Vz5uUx7ZS
BWdzXLMgDfRK6kSibByybd1k+c1gVT6/C236kDMnCreTDv/8V/pzRdZuxXNp7cve
uqP/Hdt0sQ/3RZt/SbzSGgOn0SngY5zq/CrD8fWPtDNqp29vYJhTLkQNu4f4Laga
xw0pBRdSQH4Qgpvr6bENCIJEvTVsObfKrk6hmbivcCZEnck5ublRrP1pSpX7NNsR
0rtW+BRvZ33g9XauYJNHjGZw58Yk86GIbR9yNAWvKQKBgQD9ueuL8sUraM0VGBM+
fPOEgvRJRBLbGDE/YTaHfsDMbo7MfTzMx7s88oZ4sWK5m74Q9xgzmLTAdR33+aP+
2M5sGYoMGJ80VFiU0GNLzx4b+UaQjJGimdskt7jPLtWYqisp1rIPyt1YqiW9/+7N
E7LJEaNp2dOXo2cyiGkDTSJ1+wKBgQDSl/xSVSSypfk/meKMHOm2b6uJrxjxbh3k
sbcOTtOBbnRKpiFn0qKc+hg8gVx4C2oCRARy0Cb6UdiKeAQGnKhNyDHRc6X663Nf
IhejQSzSr7YQMp+e5aokqVhNAIKUVPbBqAH3FcliugTEvSAhqrpYmRfWcL1JGAtp
fsCwfiBhPwKBgFTUDwtYHFGaFU7jILP4pJX+DlxcdQ1pNjXhc5CzcRQoNnfpuyUf
9dq3nvvFGqPBjTcw1ZeSi4P45A8fGJRAuirsEvpeVQPy2zbNwRT8Zy2lFtGHHb3G
QL0y64fhOapz4kB3tkqPYZeeM0YlxhosZs7Qpdji6do92GfNsYZYCJh3AoGBAL5D
Rln0RsWIolnN5BoGfTiwR2CLa5OkkyltTh+FJB3J9xMCaSft67C7W24+yZZDabFG
VmaMnMqHa4YezDRVENHFTw0W2RyL4J5QpN6AIcjvqyOTORdME54vyXE4RsFQyedI
rQ3ZBytFOM2uuPrxzo4BMsIw9TK2oIjdVaWfLg3tAoGBAOHwaJZ3SqdZKPVs3r2l
fBXRTaWtDHSrIACG3RWeVIbPJxMxYtsfGAXjhjnrcpbLqVxIgWqKH5uhE6zKrZSO
qX0UHybEjEqN2NBLjMjbPN7mJwYhpt/tfJQ2br4ke7HlHWlZwD1+C7/zs8xzaGhq
2xNsbYjFhnizTCv6bQHIQ64k
-----END PRIVATE KEY-----

)";

std::string_view client_crt = R"(
-----BEGIN CERTIFICATE-----
MIIDfjCCAmagAwIBAgIUQ5m7PzT+YtgC8SPFQf/zUi52t2QwDQYJKoZIhvcNAQEL
BQAwRTELMAkGA1UEBhMCQ04xIzAhBgNVBAoMGlBlb3BsZSdzIFJlcHVibGljIG9m
IENoaW5hMREwDwYDVQQDDAhDaGluYSBDQTAeFw0yMzA3MTIxMDQ0NTlaFw0zMzA3
MDkxMDQ0NTlaMEAxETAPBgNVBAoMCE15RG9tYWluMREwDwYDVQQLDAhNeURvbWFp
bjEYMBYGA1UEAwwPd3d3LmV4YW1wbGUuY29tMIIBIjANBgkqhkiG9w0BAQEFAAOC
AQ8AMIIBCgKCAQEA0Lkl91qAQCDJ3fEopoMkahWyCQ9WZxuGpqmUoA+d3mbeoqul
a2CpaZLS9ZDRIj6L+hXe3fbIvl+lhmI0Wgz+syv+ZQL+QfMtHsEESE+RaoWTgYf6
j+f2dLsZRi11eRziKiSw6N9p9NrDmv7uqok0dtgWp/XF4/jsgcNOUOml+r4g1H66
5SP7wRndF0RF2b6siGSerB6gPML8OtvlBPzTL4m/mqm2N68qW/KeHK3Npdacb0wi
dcaY/gaP38D26flDuesCUsX6ZB8Ub0GCjVxB+HHL1Wg05vDHNbCE/VXCHj/fawsV
vZcXqcfwhF5D0EQ1KNzsVVTsJInEdA76U48jxQIDAQABo2swaTAnBgNVHREEIDAe
ggtleGFtcGxlLmNvbYIPd3d3LmV4YW1wbGUuY29tMB0GA1UdDgQWBBSbn9Pz6YBS
rBscRgHyQTgbmHb7lTAfBgNVHSMEGDAWgBSz8f0p/cebANK6TyVMNithgRCiQDAN
BgkqhkiG9w0BAQsFAAOCAQEAdT+/hlG4hUHjTqwF978PoELrw9VIDV+9uob51kzq
AyZBawLy0xPlXc3Yjf3kUgWsLu2ndCmVfsIXZ9ep+Qyse9VvA2vUdQRNVfwRNIdW
tkqTImI37PFiW6QGPVhowrz//QEvO/KF1F09eSoSsSe+KhJvchoNvnwNJG2ZLxHm
azDf2lxfQGq2+z5SZGDwZiwlObyx8L/weD3jRtVZ9E1qOblUvgV0z2PXIwdJKUsC
qB2mZXBq8YxQexN0An/oKhV/SsQ094QmUvgtEETf55gDniVOL/94queC4ICqgzWr
b4zMpm6maKW34cslwKdlHUBpwVbY9O54diaearPz50NxqQ==
-----END CERTIFICATE-----

)";

/*
OpenSSL创建生成CA证书、服务器、客户端证书及密钥
https://zhuanlan.zhihu.com/p/632315595
https://blog.csdn.net/qq153471503/article/details/109524764

./bin/openssl genrsa -aes256 -out ./gen_cer/ca.key 2048
./bin/openssl req -new -sha256 -key ./gen_cer/ca.key -out ./gen_cer/ca.csr -subj "/C=CN/ST=SD/L=JN/O=QDZY/OU=www.test.com/CN=CA/emailAddress=admin@test.com"
./bin/openssl x509 -req -days 36500 -sha256 -extensions v3_ca -signkey ./gen_cer/ca.key -in ./gen_cer/ca.csr -out ./gen_cer/ca.cer
// svr
./bin/openssl genrsa -aes256 -out ./gen_cer/server.key 2048
./bin/openssl req -new -sha256 -key ./gen_cer/server.key -out ./gen_cer/server.csr -subj "/C=CN/ST=SD/L=JN/O=QDZY/OU=www.test.com/CN=SERVER/emailAddress=admin@test.com"
./bin/openssl x509 -req -days 36500 -sha256 -extensions v3_req  -CA  ./gen_cer/ca.cer -CAkey ./gen_cer/ca.key  -CAserial ./gen_cer/ca.srl  -CAcreateserial -in ./gen_cer/server.csr -out ./gen_cer/server.cer
//cli
./bin/openssl genrsa -aes256 -out ./gen_cer/client.key 2048
./bin/openssl req -new -sha256 -key ./gen_cer/client.key  -out ./gen_cer/client.csr -subj "/C=CN/ST=SD/L=JN/O=QDZY/OU=www.test.com/CN=CLIENT/emailAddress=admin@test.com"
./bin/openssl x509 -req -days 36500 -sha256 -extensions v3_req  -CA  ./gen_cer/ca.cer -CAkey ./gen_cer/ca.key  -CAserial ./gen_cer/ca.srl  -CAcreateserial -in ./gen_cer/client.csr -out ./gen_cer/client.cer

./bin/openssl dhparam -out ./gen_cer/dh.pem 2048

//asio2
../bin/openssl genrsa -des3 -out server.key 2048
../bin/openssl req -new -key server.key -out server.csr -config ../openssl.cnf
../bin/openssl genrsa -des3 -out client.key 2048
../bin/openssl req -new -key client.key -out client.csr -config ../openssl.cnf
../bin/openssl genrsa -des3 -out ca.key 2048
../bin/openssl req -new -x509 -key ca.key -out ca.crt -days 3650 -config ../openssl.cnf
../bin/openssl ca -in server.csr -out server.crt -cert ca.crt -keyfile ca.key -config ../openssl.cnf
../bin/openssl ca -in client.csr -out client.crt -cert ca.crt -keyfile ca.key -config ../openssl.cnf
../bin/openssl dhparam -out dh1024.pem 2048


// 知乎
../bin/openssl genrsa -out ca.key 2048
../bin/openssl req -new -out ca.csr -key ca.key -subj "/C=CN/O=People's Republic of China/CN=China CA" -config ../openssl.cnf
../bin/openssl x509 -req -in ca.csr -signkey ca.key -out ca.crt -days 3650 -extfile /etc/ssl/openssl.cnf -extensions v3_ca

../bin/openssl genrsa -out server.key 2048
../bin/openssl req -new -key server.key -out server.csr -subj "/O=MyDomain/OU=MyDomain/CN=www.example.com" -config ../openssl.cnf
echo "subjectAltName=DNS:example.com,DNS:www.example.com" > ./cert_extensions
../bin/openssl x509 -req -CA ca.crt -CAkey ca.key -in server.csr -out server.crt -CAcreateserial -days 3650 -extfile ./cert_extensions

../bin/openssl genrsa -out client.key 2048
../bin/openssl req -new -key client.key -out client.csr -subj "/O=MyDomain/OU=MyDomain/CN=www.example.com" -config ../openssl.cnf
../bin/openssl x509 -req -CA ca.crt -CAkey ca.key -in client.csr -out client.crt -CAcreateserial -days 3650 -extfile ./cert_extensions
*/

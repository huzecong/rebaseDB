DROP DATABASE orderDB;
CREATE DATABASE orderDB;

USE orderDB;

CREATE TABLE publisher (
  id int(10) NOT NULL,
  name char(100) NOT NULL,
  state char(2),
  PRIMARY KEY  (id)
);

CREATE TABLE book (
  id int(10) NOT NULL,
  title char(100) NOT NULL,
  authors char(200),
  publisher_id int(10) NOT NULL,
  copies int(10),
  pages int(10),
  PRIMARY KEY  (id)
);

CREATE TABLE customer (
  id int(10) NOT NULL,
  name char(25) NOT NULL,
  gender char(1),
  PRIMARY KEY  (id)
);

CREATE TABLE orders (
  customer_id int(10) NOT NULL,
  book_id int(10) NOT NULL,
  quantity int(10) NOT NULL
);



INSERT INTO book VALUES 
 (200000,'Some book','Some author',0,100,100);
INSERT INTO book VALUES 
 (200001,'Marias Diary (Plus S.)','Mark P. O. Morford',100082,5991,2530);
INSERT INTO book VALUES 
 (200002,'Standing in the Shadows','Richard Bruce Wright',101787,2900,1860);
INSERT INTO book VALUES 
 (200003,'Children of the Thunder','Carlo DEste',102928,3447,2154);
INSERT INTO book VALUES 
 (200004,'The Great Gilly Hopkins','Gina Bari Kolata',101339,39,2809);
INSERT INTO book VALUES 
 (200005,'Meine Juden--eure Juden','E. J. W. Barber',103089,206,2771);
INSERT INTO book VALUES 
 (200006,'You Can Draw a Kangaroo','Amy Tan',101850,5296,2092);

INSERT INTO customer VALUES
 (300000,'Somebody','X');
INSERT INTO customer VALUES
 (300001,'CHAD CABELLO','F');
INSERT INTO customer VALUES
 (300002,'FAUSTO VANNORMAN','F');
INSERT INTO customer VALUES
 (300003,'JO CANNADY','M');
INSERT INTO customer VALUES
 (300004,'LAWERENCE MOTE','F');
INSERT INTO customer VALUES
 (300005,'RODERICK NEVES','F');

INSERT INTO orders VALUES
 (300000, 200000, 10);
INSERT INTO orders VALUES
 (300001, 200001, 12);
INSERT INTO orders VALUES
 (300002, 200002, 14);
INSERT INTO orders VALUES
 (300003, 200003, 16);
INSERT INTO orders VALUES
 (300004, 200004, 18);
INSERT INTO orders VALUES
 (300005, 200005, 20);
INSERT INTO orders VALUES
 (300001, 200006, 22);

select name, quantity, pages
from customer, orders, book
where customer.id = orders.customer_id and
      book.id = orders.book_id;

delete from customer;
delete from orders;
delete from book;

INSERT INTO book VALUES 
 (200000,'Some book','Some author',0,100,100),
 (200001,'Marias Diary (Plus S.)','Mark P. O. Morford',100082,5991,2530),
 (200002,'Standing in the Shadows','Richard Bruce Wright',101787,2900,1860),
 (200003,'Children of the Thunder','Carlo DEste',102928,3447,2154),
 (200004,'The Great Gilly Hopkins','Gina Bari Kolata',101339,39,2809),
 (200005,'Meine Juden--eure Juden','E. J. W. Barber',103089,206,2771),
 (200006,'You Can Draw a Kangaroo','Amy Tan',101850,5296,2092),
 (200007,'The Little Drummer Girl','Robert Cowley',104382,1006,2764),
 (200008,'A Walk Through the Fire','Scott Turow',102008,8795,2543),
 (200009,'The Nursing Home Murder','David Cordingly',102866,7380,2019),
 (200010,'The Blanket of the Dark','Ann Beattie',103933,5242,1483),
 (200011,'Not Without My Daughter','David Adams Richards',101177,567,1851),
 (200012,'Introducing Halle Berry','Adam Lebor',104762,3505,1040),
 (200013,'Men Who Love Too Little','Sheila Heti',103084,6131,2770),
 (200014,'Once In a House On Fire','R. J. Kaiser',104024,4472,2876),
 (200015,'Skindeep (Pan Horizons)','Jack Canfield',100670,4898,2729),
 (200016,'A Voice Through a Cloud','Loren D. Estleman',101508,8322,1171),
 (200017,'Master Georgie: A Novel','Robert Hendrickson',102615,1448,2716),
 (200018,'Verdun (Lost Treasures)','Julia Oliver',102598,7459,1430),
 (200019,'Der Pferdefl??sterer.','John Grisham',103834,6335,2303),
 (200020,'Snowboarding to Nirvana','Toni Morrison',101085,8670,1524),
 (200021,'Boys and Girls Together','The Onion',102228,3546,1375),
 (200022,'Another Century of War?','Celia Brooks Brown',101834,2502,2468),
 (200023,'A Book of Weather Clues','J. R. Parrish',100372,6066,2053),
 (200024,'The Klutz Strikes Again','Mary-Kate &amp; Ashley Olsen',100733,8831,2687),
 (200025,'Commitments (Arabesque)','Robynn Clairday',100289,2509,1827),
 (200026,'The Third Twin: A Novel','Kathleen Duey',102335,4515,1624),
 (200027,'The Dog Who Wouldnt Be','Rich Shapero',102789,6722,2581),
 (200028,'Destined for the Throne','Michael Crichton',102197,6842,2269),
 (200029,'Year of the Roasted Ear','C.S. Lewis',100428,8147,2981),
 (200030,'The Horned Man: A Novel','ARTHUR PHILLIPS',104445,97,1774),
 (200031,'Ensayo Sobre LA Ceguera','Stephan Jaramillo',100528,8844,2441),
 (200032,'Flying High (Arabesque)','Mordecai Richler',103321,3338,1844),
 (200033,'Du entkommst mir nicht.','Eleanor Cooney',102133,3913,1433),
 (200034,'Keeping Faith : A Novel','Charlotte Link',100765,1018,2554),
 (200035,'The Gift for All People','Richard North Patterson',104940,4918,1573),
 (200036,'Deer &amp; deer hunting','Mark Salzman',104582,3657,2549),
 (200037,'Trust in Me (Arabesque)','Harper Lee',102886,8015,2077),
 (200038,'Gotham Diaries: A Novel','LAURA HILLENBRAND',102273,8397,1449),
 (200039,'The Book of the Unicorn','Barbara Kingsolver',101182,5168,1706),
 (200040,'The Multicultiboho Show','Jo Dereske',100901,7885,1141),
 (200041,'Le Poulpe : Zarmageddon','Jane Austen',104804,8188,2818),
 (200042,'Trials of Tiffany Trott','Dolores Krieger',101887,7012,1420),
 (200043,'Bob Knight: His Own Man','Anne Rivers Siddons',100146,1426,2457),
 (200044,'The Mediterranean Caper','Dean R. Koontz',102144,5405,1547),
 (200045,'A Dictionary of Symbols','Mary Higgins Clark',103812,7116,1273),
 (200046,'The Ghost and Mrs. Muir','Dean Koontz',101637,8830,1048),
 (200047,'The Well (King Penguin)','Patricia Cornwell',100657,418,1827),
 (200048,'Without a Hero: Stories','J.D. Robb',100882,2869,1029),
 (200049,'Speaking With Strangers','Maeve Binchy',103744,4551,1007),
 (200050,'101 Uses for a Dead Cat','Laura J. Mixon',103539,1492,2321),
 (200051,'The Knight and the Rose','Tim Lahaye',103425,2928,2350),
 (200052,'Departures and Arrivals','M.D. Bernie S. Siegel',101545,6307,2797),
 (200053,'The Book of the Dun Cow','Robert Penn Warren',102114,9314,2923),
 (200054,'Neils Book of the Dead','Hans Johannes Hoefer',101686,4319,1528),
 (200055,'A Secret Word : A Novel','Mark Helprin',104282,1330,1268),
 (200056,'Tabloid Dreams: Stories','O. Carol Simonton Md',103277,8482,1220),
 (200057,'The Truth About Forever','Chuck Hill',104086,9723,2829),
 (200058,'Death of Expert Witness','David Iglehart',101721,359,1403),
 (200059,'A Garden for Miss Mouse','Larry McMurtry',102973,5852,2697),
 (200060,'Dress You Up in My Love','SUZANNE FISHER STAPLES',102016,2732,2734),
 (200061,'Night Journey : A Novel','Michel Tournier',101049,2921,1890),
 (200062,'Modern American Memoirs','Carl Sagan',104651,8306,1114),
 (200063,'Gone, but Not Forgotten','Aleksandr Zinoviev',104093,4024,1387),
 (200064,'A Manhattan Ghost Story','Anne Tyler',104581,3526,2277),
 (200065,'The Lords of Discipline','Joseph Conrad',100246,3036,2375),
 (200066,'Beetles Lightly Toasted','Deepak Chopra',102797,7169,2252),
 (200067,'The Horror Hall of Fame','Thomas Hardy',104479,1810,1682),
 (200068,'Milkrun (Red Dress Ink)','Charles Noland',100586,7749,2642),
 (200069,'Once Upon a Potty : His','Valerie Frankel',102845,2083,1511),
 (200070,'Voyage: A Novel of 1896','Benjamin Hoff',100835,5554,2363),
 (200071,'The Fairest Among Women','Niccolo Machiavelli',101291,5001,2862),
 (200072,'Win Strk Schol: Lisa PB','H. Jackson Brown',101104,5446,1713),
 (200073,'How to talk to your cat','Robert A. Heinlein',101555,4077,2765),
 (200074,'Home Buying for Dummies','PHILIP PULLMAN',103656,3545,2885),
 (200075,'The Cat Who...Companion','Anna Sewell',103946,5574,2134),
 (200076,'A Manhattan Ghost Story','MICHAEL ONDAATJE',102039,3218,1375),
 (200077,'The Revelation: A Novel','Sandra Levy Ceren',100607,3778,2957),
 (200078,'Shadows (Smallville #5)','P.J. ORourke',101628,4033,2380),
 (200079,'Playgrounds of the Mind','Mike Gayle',100288,8121,2330),
 (200080,'Escape from the Kitchen','Stel Pavlou',104317,8336,2266),
 (200081,'Master Georgie: A Novel','Sarah Payne Stuart',100280,6938,1645),
 (200082,'Infancia de Un Jefe, La','Dan Quayle',103599,1786,1592),
 (200083,'Panther in the Basement','Donald F. Kettl',104937,6136,1558),
 (200084,'Nicholas at the Library','DAVID FRUM',101238,8860,2148),
 (200085,'Auf Gedeih und Verderb.','Louis Lamour',101698,6095,1655),
 (200086,'Object Lessons: A Novel','J.D. Salinger',103260,9806,1270),
 (200087,'Les bruines de Lanester','J. R. R. Tolkien',100645,7318,1708),
 (200088,'A House for Hermit Crab','John Berendt',102706,2005,1333),
 (200089,'All the Sweet Tomorrows','Jennifer Crusie',103624,4066,2210),
 (200090,'1000 Forbidden Pictures','Jane Heller',104329,6966,2990),
 (200091,'T??dliche Versuchung.','Michael Rips',102533,6794,2538),
 (200092,'A Suitable Boy: A Novel','Simon Mawer',100299,6228,1847),
 (200093,'Arthur: King of Britain','William Abrahams',101328,1533,2030),
 (200094,'Autumn of the Patriarch','Robert T. Kiyosaki',100707,8673,2577),
 (200095,'Canapes for the Kitties','Ken Follett',103093,1446,1444),
 (200096,'Diet for a Small Planet','John F. Love',103158,6576,2427),
 (200097,'Beneath a Midnight Moon','Robert G. Allen',104252,5814,1742),
 (200098,'Deadlines and Datelines','LOUIS DE BERNIERES',101756,8459,1592),
 (200099,'Gulac War (Sobs, No 6)','Pam Proctor',100546,9538,2544),
 (200100,'Anyone Can Have a Happy',null,103343,3358,2213);

INSERT INTO orders VALUES
 (300000,200000,100),
 (300094,200161,0),
(300178,200165,10),
(300158,200033,16),
(300114,200111,20),
(300111,200190,16),
(300076,200006,11),
(300032,200048,11),
(300151,200122,18),
(300113,200021,5),
(300068,200126,4),
(300132,200139,7),
(300108,200067,17),
(300020,200079,5),
(300035,200001,8),
(300115,200108,18),
(300096,200164,4),
(300065,200121,18),
(300089,200064,1),
(300065,200197,7),
(300185,200199,8),
(300098,200071,6),
(300017,200146,14),
(300061,200079,2),
(300165,200050,17),
(300087,200103,20),
(300100,200004,14),
(300155,200091,11),
(300062,200197,14),
(300127,200113,5),
(300130,200094,20),
(300139,200162,12),
(300002,200132,19),
(300011,200064,15),
(300146,200159,17),
(300183,200089,19),
(300072,200059,4),
(300013,200170,18),
(300008,200117,16),
(300021,200060,6),
(300157,200093,3),
(300122,200111,10),
(300091,200090,4),
(300137,200101,19),
(300097,200076,11),
(300057,200075,14),
(300035,200155,9),
(300073,200110,15),
(300140,200025,7),
(300173,200172,2),
(300036,200037,1),
(300094,200094,15),
(300066,200138,12),
(300107,200047,3),
(300173,200137,19),
(300120,200033,18),
(300078,200097,8),
(300121,200080,11),
(300100,200107,15),
(300082,200111,14),
(300128,200153,7),
(300127,200071,4),
(300132,200005,10),
(300011,200164,7),
(300062,200148,17),
(300194,200059,1),
(300096,200091,20),
(300137,200114,1),
(300000,200160,3),
(300071,200071,17),
(300063,200173,15),
(300116,200103,2),
(300172,200135,11),
(300033,200040,9),
(300162,200114,16),
(300062,200038,20),
(300060,200001,7),
(300062,200011,20),
(300199,200090,15),
(300099,200104,15),
(300123,200171,12),
(300037,200145,3),
(300091,200054,2),
(300069,200029,16),
(300003,200014,17),
(300007,200066,18),
(300113,200001,20),
(300128,200170,14),
(300191,200097,9),
(300101,200075,7),
(300168,200140,16),
(300166,200159,0),
(300138,200174,2),
(300041,200135,5),
(300164,200030,4),
(300004,200150,9),
(300105,200093,15),
(300161,200127,18),
(300008,200160,19),
(300133,200180,19),
(300091,200067,17),
(300043,200049,14),
(300179,200185,19),
(300086,200120,11),
(300136,200028,4),
(300113,200038,10),
(300017,200063,4),
(300197,200028,16),
(300133,200171,7),
(300083,200044,19),
(300003,200145,10),
(300122,200140,4),
(300180,200040,13),
(300117,200190,15),
(300136,200194,19),
(300043,200110,19),
(300024,200110,4),
(300011,200147,20),
(300164,200136,3),
(300152,200097,1),
(300121,200150,20),
(300030,200103,1),
(300135,200200,20),
(300085,200086,2),
(300094,200063,18),
(300118,200172,18),
(300098,200140,13),
(300008,200150,0),
(300116,200164,14),
(300023,200177,5),
(300039,200128,5),
(300119,200115,20),
(300095,200041,11),
(300115,200157,13),
(300039,200080,5),
(300141,200032,8),
(300014,200154,16),
(300002,200044,3),
(300047,200021,4),
(300089,200017,2),
(300047,200129,4),
(300133,200048,16),
(300032,200116,10),
(300107,200105,3),
(300029,200158,5),
(300114,200013,1),
(300100,200044,9),
(300050,200167,11),
(300103,200137,16),
(300098,200043,11),
(300153,200116,5),
(300061,200109,16),
(300061,200066,16),
(300024,200046,3),
(300105,200090,8),
(300133,200035,3),
(300005,200191,9),
(300124,200110,19),
(300077,200135,17),
(300131,200117,17),
(300149,200134,3),
(300001,200099,17),
(300200,200146,13),
(300159,200023,14),
(300049,200125,13),
(300020,200125,0),
(300055,200132,0),
(300186,200070,6),
(300057,200171,15),
(300089,200048,19),
(300165,200147,7),
(300143,200096,12),
(300003,200137,19),
(300102,200033,7),
(300115,200017,3),
(300174,200067,0),
(300045,200151,6),
(300016,200159,12),
(300052,200095,10),
(300147,200159,2),
(300010,200187,2),
(300040,200175,14),
(300197,200091,16),
(300088,200092,3),
(300024,200192,0),
(300097,200121,2),
(300112,200159,10),
(300191,200107,11),
(300068,200193,1),
(300103,200024,19),
(300093,200091,0),
(300066,200179,1),
(300038,200152,14),
(300041,200060,17),
(300123,200144,7),
(300108,200150,10),
(300055,200045,0),
(300013,200197,20),
(300003,200007,3),
(300005,200007,3),
(300167,200036,17);

INSERT INTO customer VALUES
 (300000,'Somebody','X'),
 (300001,'CHAD CABELLO','F'),
 (300002,'FAUSTO VANNORMAN','F'),
 (300003,'JO CANNADY','M'),
 (300004,'LAWERENCE MOTE','F'),
 (300005,'RODERICK NEVES','F'),
 (300006,'JACOB LEDGER','M'),
 (300007,'WALKER JOLIN','M'),
 (300008,'SELINA TAULBEE','F'),
 (300009,'BRUCE BARTHOLOMEW','F'),
 (300010,'PRINCE HALLETT','M'),
 (300011,'MARGERT MITCHELL','F'),
 (300012,'WARNER BOWEN','F'),
 (300013,'VENNIE ANDRESS','F'),
 (300014,'EUSEBIA PEGG','M'),
 (300015,'DAN LINEBERGER','F'),
 (300016,'MAGEN MCARTHUR','F'),
 (300017,'BABETTE HEATH','F'),
 (300018,'FIDELA BENNY','F'),
 (300019,'KARY GUERTIN','F'),
 (300020,'LUANNE TIMBERLAKE','F'),
 (300021,'CLAUDIA WAITES','F'),
 (300022,'DARIN CORDLE','M'),
 (300023,'JOSHUA KNAUS','M'),
 (300024,'TAWNA NUSBAUM','F'),
 (300025,'HALINA NIELSON','M'),
 (300026,'TODD BOUDREAUX','M'),
 (300027,'HAYDEN JOSHUA','M'),
 (300028,'LUCIEN FELICE','M'),
 (300029,'SONJA SAYLORS','F'),
 (300030,'SACHA TIGHE','M'),
 (300031,'BLAKE MCCANN','M'),
 (300032,'ELVIRA TUCCI','F'),
 (300033,'NOE CRITTENDEN','F'),
 (300034,'LONNIE SALAZAR','F'),
 (300035,'AJA TRIBBLE','M'),
 (300036,'NIGEL TROTMAN','M'),
 (300037,'LINCOLN RANCK','M'),
 (300038,'KIP FRISCH','M'),
 (300039,'BOBBY THORPE','F'),
 (300040,'SAMIRA GASKIN','M'),
 (300041,'WARD FLINCHUM','F'),
 (300042,'DANIAL AUBREY','M'),
 (300043,'MARLIN HOLDER','F'),
 (300044,'RACHEL CARRON','F'),
 (300045,'LAKIESHA KRANTZ','F'),
 (300046,'WILTON AMICO','F'),
 (300047,'CHRISTOPHER KIMBLER','F'),
 (300048,'LINDSEY BACCHUS','F'),
 (300049,'PAMELIA MASSEY','F'),
 (300050,'PHIL LIKENS','F'),
 (300051,'LOUIE BRESHEARS','M'),
 (300052,'DERRICK OLESON','F'),
 (300053,'TABATHA CROSKEY','M'),
 (300054,'RAYMOND HELSLEY','M'),
 (300055,'SHELLIE STOKLEY','F'),
 (300056,'BEBE KREIDER','F'),
 (300057,'RUSTY PITTARD','F'),
 (300058,'ISAIAH HAUSMANN','M'),
 (300059,'CHADWICK JENT','F'),
 (300060,'VERDIE TILLER','M'),
 (300061,'KIA DIMATTIA','F'),
 (300062,'MARC RIZZO','M'),
 (300063,'TYREE TEEMS','F'),
 (300064,'ELAYNE RAWSON','F'),
 (300065,'RUSTY FELT','F'),
 (300066,'OPHELIA LAMBERSON','F'),
 (300067,'ROBENA WATT','M'),
 (300068,'CYRIL THOME','F'),
 (300069,'WILBERT ARCHIBALD','F'),
 (300070,'REY RUPE','M'),
 (300071,'ALMEDA GUTIERREZ','F'),
 (300072,'JED RIOS','F'),
 (300073,'DEWITT CONERLY','M'),
 (300074,'HERIBERTO DREHER','F'),
 (300075,'MARQUIS KEEFE','F'),
 (300076,'TONYA SIGLER','F'),
 (300077,'DEVONA BAXLEY','M'),
 (300078,'CURT REAM','F'),
 (300079,'ANJELICA ALEJO','F'),
 (300080,'ARLINDA VOLL','F'),
 (300081,'NARCISA HATFIELD','M'),
 (300082,'VIKI WEINSTOCK','M'),
 (300083,'LYDA MOYERS','F'),
 (300084,'AUGUSTINE TRAINER','F'),
 (300085,'RANDY ROGOWSKI','F'),
 (300086,'BENITO MOBERG','F'),
 (300087,'LENNY PETERSEN','F'),
 (300088,'EDDIE DEYOUNG','F'),
 (300089,'BONNIE PARKE','F'),
 (300090,'MARYLAND RUETER','F'),
 (300091,'DARIUS MCLAIN','M'),
 (300092,'GAYE MUNOS','F'),
 (300093,'MYUNG BERRYHILL','M'),
 (300094,'ABDUL POINDEXTER','F'),
 (300095,'ROMAINE DOSSETT','M'),
 (300096,'GAYLE DEMOSS','M'),
 (300097,'SHELLEY MAZZONE','F'),
 (300098,'JUNIOR MANZELLA','F'),
 (300099,'PAULITA SEYBERT','F'),
 (300100,'MIQUEL PATTI','F');

INSERT INTO publisher VALUES
 (104821,'Alberta Teachers Association','CA'),
 (104823,'OUP Australia and New Zealand','CA'),
 (104824,'Lothrop, Lee &amp; Shepard Co','CA'),
 (104828,'English Heritage Publications','CA'),
 (104836,'distributed by Pantheon Books','CA'),
 (104837,'Alpha Publishing Company (KY)','CA'),
 (104842,'Book Buddy Publishing Company','CA'),
 (104843,'Contender Entertainment Group','CA'),
 (104845,'Badlands Natural History Assn','CA'),
 (104846,'New Leaf Distributing Company','CA'),
 (104847,'Instructional Fair/Ts Denison','CA'),
 (104848,'Santa &amp; the Christ Child.','CA'),
 (104849,'Management Information Source','CA'),
 (104851,'Tweetyjill Publications, Inc.','CA'),
 (104854,'Bantam Doubleday Dell Pub (J)','CA'),
 (104855,'Carlisle Press - Walnut Creek','CA'),
 (104856,'Nymphenburger Verlagshandlung','CA'),
 (104860,'Centax Books and Distribution','CA'),
 (104861,'Best of the Bridge Publishing','CA'),
 (104862,'Webster Division, McGraw-Hill','CA'),
 (104867,'Starlight Writer Publications','CA'),
 (104868,'Alpha Omega Dental Fraternity','CA'),
 (104869,'Frederick Ungar Publishing Co','CA'),
 (104870,'William S. Konecky Associates','CA'),
 (104872,'Greey de Pencier Publications','CA'),
 (104874,'Byron Preiss Multimedia Books','CA'),
 (104875,'Seven Arrows Publishing, Inc.','CA'),
 (104876,'Wissenschaftliche Verlagsges.','CA'),
 (104881,'Educational Media Corporation','CA'),
 (104882,'Bufflehead Books &amp; Prints','CA'),
 (104884,'Crossquarter Publishing Group','CA'),
 (104885,'Saint Johns University Press','CA'),
 (104886,'New American Library / Signet','CA'),
 (104887,'Intext Educational Publishers','CA'),
 (104888,'Hodder Moa Beckett Publishers','CA'),
 (104890,'Harvard Educational Pub Group','CA'),
 (104891,'National Gallery of Australia','CA'),
 (104892,'Turnbull &amp; Willoughby Pub','CA'),
 (104893,'Mcclelland &amp; Stewart Ltd.','CA'),
 (104894,'National Storytelling Network','CA'),
 (104897,'American Planning Association','CA'),
 (104898,'Phi Delta Kappa International','CA'),
 (104900,'A Crossings Book Club Edition','CA'),
 (104902,'Guildhall Publishers, Limited','CA'),
 (104903,'North Star Press of St. Cloud','CA'),
 (104904,'Amer Anti-Vivisection Society','CA'),
 (104905,'Suma de Letras Suma de Letras','CA'),
 (104906,'Alaska Native Language Center','CA'),
 (104908,'Islamic Educational Center of','CA'),
 (104909,'Concept Systems, Incorporated','CA'),
 (104912,'Txalaparta Argitaletxea, S.L.','CA'),
 (104914,'Affholderbach &amp; Strohmann','CA'),
 (104915,'Random House Audio Price-Less','CA'),
 (104916,'Thirty Seven Books Publishing','CA'),
 (104917,'United States Naval Institute','CA'),
 (104920,'American Technical Publishers','CA'),
 (104921,'Museum of Jurassic Technology','CA'),
 (104923,'Lightlines Publishing Company','CA'),
 (104924,'Distributed by H. Holt and Co','CA'),
 (104925,'Prima Pub. and Communications','CA'),
 (104927,'West Virginia Univ College of','CA'),
 (104928,'Southern California Committee','CA'),
 (104930,'Verlag Edition Text u. Kritik','CA'),
 (104934,'Spiritualists National Union','CA'),
 (104935,'Annie Lee Taylor Publications','CA'),
 (104936,'Chapel &amp; Croft Publishing','CA'),
 (104937,'Crown Publishers/random House','CA'),
 (104939,'Verlag Katholisches Bibelwerk','CA'),
 (104941,'Webb &amp; Bower (Publishing)','CA'),
 (104942,'Random House Value Publishing','CA'),
 (104944,'Harper Mass Market Paperbacks','CA'),
 (104945,'MacMillan Publishing Company.','CA'),
 (104947,'Dearborn Financial Publishing','CA'),
 (104952,'Knopf Books for Young Readers','CA'),
 (104953,'Sagebrush Education Resources','CA'),
 (104954,'University of Minnesota Press','CA'),
 (104958,'Scott Foresman (Pearson K-12)','CA'),
 (104959,'Financial Times/Prentice Hall','CA'),
 (104963,'Christian Publishing Services','CA'),
 (104964,'University of Wisconsin Press','CA'),
 (104966,'Ulverscroft Large Print Books','CA'),
 (104969,'Astro Communications Services','CA'),
 (104976,'Ellen C Temple Publishing Inc','CA'),
 (104977,'Dorchester Publishing Company','CA'),
 (104982,'W.W. Norton &amp; Company Ltd','CA'),
 (104989,'Golden Books Adult Publishing','CA'),
 (104991,'World Heritage Publishing Inc','CA'),
 (104992,'China Books &amp; Periodicals','CA'),
 (104993,'Dow Jones &amp; Company, Inc.','CA'),
 (104994,'Fox Chapel Publishing Company','CA'),
 (104996,'W.h. Smith And Son Publishers','CA'),
 (104997,'Transworld Publishers Limited','CA');

queryplans on;

select name, quantity, pages
from customer, orders, book
where customer.id = orders.customer_id and
      book.id = orders.book_id;

EXIT;


select name, quantity, pages
from customer, orders, book
where customer.id = orders.customer_id and
      book.id = orders.book_id and
      book.id > 200010 and
      orders.customer_id > 300010;

select *
from customer, publisher, orders, book
where customer.gender = 'F' and
      customer.id = orders.customer_id and
      pages > 100 and
      state = 'CA' and
      publisher.id = book.publisher_id;

UPDATE customer SET gender = 'M' WHERE gender = 'F';

SELECT name, quantity, pages
FROM customer, orders, book
WHERE customer.id = orders.customer_id AND
      book.id = orders.book_id;

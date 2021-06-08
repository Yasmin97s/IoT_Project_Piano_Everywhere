// @dart=2.9
import 'package:cloud_firestore/cloud_firestore.dart';
import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:flutter_midi/flutter_midi.dart';
import 'package:flutterfire_samples/res/custom_colors.dart';
import 'package:tonic/tonic.dart';
import 'package:firebase_auth/firebase_auth.dart';

import 'feedback.dart';

List<String> pressedToneList = [];

/*
void main() => runApp(MyApp());

class MyApp extends StatefulWidget {
  @override
  _MyAppState createState() {
    return new _MyAppState();
  }
}
*/


class ClassicalPiano extends StatefulWidget {
  static const routeName = '/ClassicalPiano';
  const ClassicalPiano({Key key,  User user})
      : _user = user,
        super(key: key);

  final User _user;
  @override
  ClassicState createState() => ClassicState();
}



class ClassicState extends State<ClassicalPiano> {
  //double get keyWidth => 50 + (50 * _widthRatio);
  double get keyWidth => 100 + (200 * _widthRatio);
  double _widthRatio = 0.0;
  bool _showLabels = true;
  User _user;

  @override
  initState()  {
     FlutterMidi().unmute();
    rootBundle.load("assets/sounds/Piano.sf2").then((sf2) {
      FlutterMidi().prepare(sf2: sf2, name: "Piano.sf2");
    });
     _user = widget._user;
    super.initState();
  }

  String _dropDownValue="Pick a student";

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Flutter Piano',
      theme: ThemeData.dark(),
      home: Scaffold(
          appBar: AppBar(title: Text("Flutter Piano"),
            actions: <Widget>[StreamBuilder<QuerySnapshot>(
              stream: FirebaseFirestore.instance.collection('Students').snapshots(),
              builder: (BuildContext context, AsyncSnapshot<QuerySnapshot> snapshot) {
                if (snapshot.hasError) {
                  return Text('Something went wrong');
                }


                List<String> uns=[];
                final posts = snapshot.data.docs.asMap();
                for(var v in posts.values) {
                  uns.add(v["studentEmail"]);
                }

                return DropdownButton<String>
                  (
                  hint: _dropDownValue == null
                      ? Text('Dropdown')
                      : Text(
                    _dropDownValue,
                    style: TextStyle(fontSize: 16,
                        letterSpacing: 0.5,
                        color: CustomColors.firebaseOrange),
                  ),

                  style: TextStyle(color: CustomColors.firebaseOrange),
                  items: uns.map(
                        (val) {
                      return DropdownMenuItem<String>(
                        value: val,
                        child: Text(val),
                      );
                    },
                  ).toList(),
                  onChanged: (val) {
                    setState(
                          () {
                        _dropDownValue = val;
                      },
                    );
                  },
                );
              },
            ),

              IconButton(
                icon: Icon(
                  Icons.send,
                  color: CustomColors.firebaseOrange,
                ),
                onPressed: () async {
                  // do something
                  if(_dropDownValue != "Pick a student" && pressedToneList.length != 0) {
                    final FirebaseFirestore _firebase = FirebaseFirestore
                        .instance;



                    String str = widget._user.uid;

                    String currentDoc = '';
                    List<int> student = [];
                    await _firebase.collection('Lessons').add(
                        {
                          "teacherKeys": pressedToneList,
                          "teacherUid": str,
                          "studentKeys": student,
                          "studentEmail": _dropDownValue,
                          "isBegin": false,
                          "time": FieldValue.serverTimestamp()
                        }).then((value) {
                      currentDoc = value.id;
                    });
                    //"teacherUid": str

                    pressedToneList.clear();
                    Navigator.push(
                      context,
                      MaterialPageRoute(builder: (context) =>
                          feedbackScreen(user: _user, docId: currentDoc,)),
                    );
                  }
                },
              ),
              IconButton(
                icon: Icon(
                  Icons.add,
                  color: CustomColors.firebaseNavy,
                ),
                onPressed: () async {
                  // do something
                  pressedToneList.removeLast();
                },
              ),
              IconButton(
                icon: Icon(
                  Icons.subdirectory_arrow_left,
                  color: Colors.white,
                ),
                onPressed: () async {
                  // do something
                  pressedToneList.removeLast();
                },
              ),
              IconButton(
                icon: Icon(
                  Icons.home,
                  color: Colors.white,
                ),
                onPressed: () async {
                  // do something
                  pressedToneList.clear();
                  Navigator.pop(context);
                },
              )],),

          drawer: Drawer(
              child: SafeArea(

                  child: ListView(children: <Widget>[
                    Container(height: 20.0),
                    ListTile(title: Text("Change Width")),
                    Slider(
                        activeColor: Colors.redAccent,
                        inactiveColor: Colors.white,
                        min: 0.0,
                        max: 1.0,
                        value: _widthRatio,
                        onChanged: (double value) =>
                            setState(() => _widthRatio = value)),
                    Divider(),
                    ListTile(
                        title: Text("Show Labels"),
                        trailing: Switch(
                            value: _showLabels,
                            onChanged: (bool value) =>
                                setState(() => _showLabels = value))),
                    Divider(),
                  ]),
                  )),
          body: ListView.builder(
            itemCount: 1,
            controller: ScrollController(initialScrollOffset: 1200.0),
            scrollDirection: Axis.horizontal,
            itemBuilder: (BuildContext context, int index) {
              final int i = 60;    //i=70 A#6 - A7
              return Container(
                child: _getOctave(i),
              );
            },
          )),
    );
  }

  Widget _getOctave(int i) {
    return SafeArea(
      child: Stack(children: <Widget>[
        Row(mainAxisSize: MainAxisSize.min, children: <Widget>[
          _buildKey(24 + i, false),
          _buildKey(26 + i, false),
          _buildKey(28 + i, false),
          _buildKey(29 + i, false),
          _buildKey(31 + i, false),
          _buildKey(33 + i, false),
          _buildKey(35 + i, false),
          _buildKey(36 + i, false),
        ]),
        Positioned(
            left: 0.0,
            right: 160.0,
            bottom: 270,
            top: 0.0,
            child: Row(
                mainAxisAlignment: MainAxisAlignment.spaceBetween,
                mainAxisSize: MainAxisSize.min,
                children: <Widget>[
                  Container(width: keyWidth * .5),
                  _buildKey(25 + i, true),
                  _buildKey(27 + i, true),
                  Container(width: keyWidth),
                  _buildKey(30 + i, true),
                  _buildKey(32 + i, true),
                  _buildKey(34 + i, true),
                  Container(width: keyWidth * .5),
                ])),
      ]),
    );
  }

  Widget _buildKey(int midi, bool accidental) {
    final pitchName = Pitch.fromMidiNumber(midi).toString();

    final pianoKey = Stack(
      children: <Widget>[
        Semantics(
            button: true,
            hint: pitchName,
            child: Material(
                borderRadius: borderRadius,
                color: accidental ? Colors.black : Colors.white,
                child: InkWell(

                  highlightColor: Colors.grey,
                  onTap: () {
                              /*0: "C6"
                                1:  "C♯6"
                                2: "D6"
                                3: "D♯6"
                                4: "E6"
                                5: "F6"
                                6 :"F♯6"
                                7: "G6"
                                8: "A6"
                                9: "G♯6"
                                10: "A♯6"
                                11: "B6"
                                12 : "C7"*/
                    String str="";
                    switch (pitchName) {
                      case "C6":
                        str ="a";
                        break;

                      case "C♯6":
                        str ="b";
                        break;

                      case "D6":
                        str ="c";
                        break;

                      case "D♯6":
                        str ="d";
                        break;

                      case "E6":
                        str ="e";
                        break;

                      case "F6":
                        str ="f";
                        break;

                      case "F♯6": //a b c d e f g h i j
                        str ="g";
                        break;
                      case "G6":
                        str ="h";
                        break;

                      case "G♯6":
                        str ="i";
                        break;

                      case "A6":
                        str ="j";
                        break;
                      case "A♯6":
                        str ="k";
                        break;
                      case "B6":
                        str ="l";
                        break;
                      case "C7":
                        str ="m";
                        break;
                    }
                    pressedToneList.add(str);
                  },
                  onTapDown: (_) => FlutterMidi().playMidiNote(midi: midi),
                ))),
        Positioned(
            left: 0.0,
            right: 0.0,
            bottom: 20.0,
            child: _showLabels
                ? Text(pitchName,
                textAlign: TextAlign.center,
                style: TextStyle(
                    color: !accidental ? Colors.black : Colors.white))
                : Container()),
      ],
    );
    if (accidental) {
      return Container(
          width: keyWidth,
          margin: EdgeInsets.symmetric(horizontal: 2.0),
          padding: EdgeInsets.symmetric(horizontal: keyWidth * .1),
          child: Material(
              elevation: 6.0,
              borderRadius: borderRadius,
              shadowColor: Color(0x802196F3),
              child: pianoKey));
    }
    return Container(
        width: keyWidth,
        child: pianoKey,
        margin: EdgeInsets.symmetric(horizontal: 2.0));
  }
}

const BorderRadiusGeometry borderRadius = BorderRadius.only(
    bottomLeft: Radius.circular(10.0), bottomRight: Radius.circular(10.0));

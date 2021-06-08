// import 'package:audioplayers/audio_cache.dart';
// import 'package:cloud_firestore/cloud_firestore.dart';
// import 'package:flutter/services.dart';
// import 'package:flutter/material.dart';
// import 'package:firebase_auth/firebase_auth.dart';
// import 'package:flutterfire_samples/screens/feedback.dart';
// import 'package:flutterfire_samples/res/custom_colors.dart';
//
//
// int c=0;
// List<String> pressedToneList = [];
//
// class piano extends StatefulWidget {
//   static const routeName = '/piano';
//   const piano({Key? key, required User user})
//       : _user = user,
//         super(key: key);
//
//   final User _user;
//
//   @override
//   _PianoState createState() => _PianoState();
// }
//
// class _PianoState extends State<piano> {
//
//   static AudioCache player = AudioCache();
//   late User _user;
//
//   @override
//
//   void initState() {
//     player.loadAll(
//         ['note1.wav', 'note2.wav', 'note3.wav', 'note4.wav', 'note5.wav' , 'note6.wav', 'note7.wav']);
//     _user = widget._user;
//     super.initState();
//   }
//
//   Widget myTone(Color myColor, String myNumber) {
//     return Expanded(
//       child: FlatButton(
//           color: myColor,
//           onPressed: () async {
//             pressedToneList.add(myNumber);
//             print(pressedToneList);
//             setState(() {
//               player.play('note'+myNumber+'.wav');
//
//             });
//           },
//           child:  Text(
//           '.',
//           style: TextStyle(
//               fontSize: 1)
//       )
//       )
//     );
//   }
//   String _dropDownValue="Pick a student";
//
//   @override
//   Widget build(BuildContext context)
//   {
//     // Set landscape orientation
//     SystemChrome.setPreferredOrientations([
//       DeviceOrientation.landscapeLeft,
//       DeviceOrientation.landscapeRight,
//     ]);
//     return MaterialApp(
//       home: Scaffold(
//         appBar: AppBar(
//           backgroundColor: CustomColors.firebaseNavy,
//           title: Text('Enjoy, ' + widget._user.displayName! ),
//           actions: <Widget>[
//             StreamBuilder<QuerySnapshot>(
//               stream: FirebaseFirestore.instance.collection('Students').snapshots(),
//               builder: (BuildContext context, AsyncSnapshot<QuerySnapshot> snapshot) {
//                 if (snapshot.hasError) {
//                   return Text('Something went wrong');
//                 }
//
//
//                 List<String> uns=[];
//                 final posts = snapshot.data!.docs.asMap();
//                 for(var v in posts.values) {
//                   uns.add(v["username"]);
//                 }
//
//                 return DropdownButton<String>
//                   (
//                   hint: _dropDownValue == null
//                       ? Text('Dropdown')
//                       : Text(
//                     _dropDownValue,
//                     style: TextStyle(fontSize: 16,
//                         letterSpacing: 0.5,
//                         color: CustomColors.firebaseOrange),
//                   ),
//
//                   style: TextStyle(color: CustomColors.firebaseOrange),
//                   items: uns.map(
//                         (val) {
//                       return DropdownMenuItem<String>(
//                         value: val,
//                         child: Text(val),
//                       );
//                     },
//                   ).toList(),
//                   onChanged: (val) {
//                     setState(
//                           () {
//                         _dropDownValue = val!;
//                       },
//                     );
//                   },
//                 );
//               },
//             ),
//
//             IconButton(
//               icon: Icon(
//                 Icons.send,
//                 color: CustomColors.firebaseOrange,
//               ),
//               onPressed: () async {
//                 // do something
//                 if(_dropDownValue != "Pick a student" && pressedToneList.length != 0) {
//                   final FirebaseFirestore _firebase = FirebaseFirestore
//                       .instance;
//
//
//
//                   String str = widget._user.uid;
//                   c++;
//                   String currentDoc = '';
//                   List<int> student = [];
//                   await _firebase.collection('Lessons').add(
//                       {
//                         "teacherKeys": pressedToneList,
//                         "teacherUid": str,
//                         "studentKeys": student,
//                         "studentUsername": _dropDownValue,
//                         "time": FieldValue.serverTimestamp()
//                       }).then((value) {
//                     currentDoc = value.id;
//                   });
//                   //"teacherUid": str
//
//                   pressedToneList.clear();
//                   Navigator.push(
//                     context,
//                     MaterialPageRoute(builder: (context) =>
//                         feedbackScreen(user: _user, docId: currentDoc,)),
//                   );
//                 }
//               },
//             ),
//             IconButton(
//               icon: Icon(
//                 Icons.add,
//                 color: CustomColors.firebaseNavy,
//               ),
//               onPressed: () async {
//                 // do something
//                 pressedToneList.removeLast();
//               },
//             ),
//             IconButton(
//               icon: Icon(
//                 Icons.subdirectory_arrow_left,
//                 color: Colors.white,
//               ),
//               onPressed: () async {
//                 // do something
//                 pressedToneList.removeLast();
//               },
//             ),
//             IconButton(
//               icon: Icon(
//                 Icons.home,
//                 color: Colors.white,
//               ),
//               onPressed: () async {
//                 // do something
//                 Navigator.pop(context);
//               },
//             )
//
//
//
//
//           ],
//         ),
//         body: SafeArea(
//           child: Row(
//             crossAxisAlignment: CrossAxisAlignment.stretch,
//             children: <Widget>[
//               myTone(CustomColors.firebaseNavy, '1'),
//               myTone(CustomColors.firebaseOrange, '2'),
//               myTone(CustomColors.firebaseNavy, '3'),
//               myTone(CustomColors.firebaseOrange, '4'),
//               myTone(CustomColors.firebaseNavy, '5'),
//               myTone(CustomColors.firebaseOrange, '6'),
//               myTone(CustomColors.firebaseNavy, '7'),
//             ],
//           ),
//         ),
//       ),
//     );
//   }
// }
//
//
//
//
//

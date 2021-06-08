import 'package:flutter/material.dart';
import 'package:flutterfire_samples/res/custom_colors.dart';

class AppBarTitle extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return Row(
      crossAxisAlignment: CrossAxisAlignment.start,
      mainAxisSize: MainAxisSize.min,
      children: [
        Image.asset(
          'assets/piano.png',
          height: 20,
        ),
        SizedBox(width: 8),
        Text(
          'Piano',
          style: TextStyle(
            color: CustomColors.firebaseYellow,
            fontSize: 18,
          ),
        ),
        Text(
          ' Everywhere',
          style: TextStyle(
            color: CustomColors.firebaseOrange,
            fontSize: 18,
          ),
        ),
        Text(
          ' App',
          style: TextStyle(
            color: CustomColors.firebaseGrey,
            fontSize: 18,
          ),
        ),
      ],
    );
  }
}

# Quest Name
Authors: Joe Rossi, Isabella Kuhl, Laura Reeve

09-19-2019

## Summary



## Evaluation Criteria



## Solution Design



## Investigative Question
How can you synchronize multiple ESP clocks with each other? Elaborate

If we want to synchronize multiple clocks with each other, the two best protocols to use would be Network Time Protocol (NTP) or Precision Time Protocol (PTP). PTP is incredibly accurate due to hardware timestamping, using multiple timestamps to sync a slave clock to a master clock. NTP is slightly less accurate but does not require hardware timestamping. In the ESP-32 specifications, they claim to have PTP hardware support, so PTP could be an option, but unless extremely high precision is necessary, NTP is simpler to implement. To start, we can set up a LAN server that all of the ESP devices can use to communicate. Both of the protocols send and recieve timestamps to establish a common time. NTP uses a hierarchical structure to establish proper time (generally UTC), but if we want these clocks to synchronize without using UTC, we can establish our own server times. Then all of the clocks will poll these higher clocks by sending and recieving timestamps (via UDP), using these to calculate the offset and delay and update its time to match up with those higher level clocks. 

One of the potential issues that we'd face would be knowing which clock(s) from the group to use to set our server clock values. If our goal is to use a user-inputted time to sync all of the clocks, that could be used as our base server clocktime. Otherwise, finding the mean of the clock times (excluding outliers) using, for example, the Berkeley algorithm, which doesn't require any seed time and simply finds an average time across multiple clocks and polls the slave clocks. Then we could use this time as the server time for our NTP implementation.

## Sketches and Photos
<center><img src="./images/example.png" width="70%" /></center>  
<center> </center>


## Supporting Artifacts
- [Link to repo]()
- [https://drive.google.com/file/d/19ph8pJgsHekSAIAexwMMzVPk7IlnPV5K/view?usp=sharing]()


## References

-----

## Reminders

- Video recording in landscape not to exceed 90s
- Each team member appears in video
- Make sure video permission is set accessible to the instructors
- Repo is private

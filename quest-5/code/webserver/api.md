# Security Central API


## Fob Access Log

Used to query the database for fob access records.

**URL** : `/fob-access/log/`

**Method** : `GET`

**Auth required** : NO

**Query Parameters**

```
name: string
    Filter by a specific person's name

fob_id: int
    Filter by a specific fob

loc: string
    Filter by location

since: Epoch seconds


until: Epoch seconds
```


### Success Response

**Code** : `200 OK`

**Content example**

```json
{
    "access_log": [
         {
            "fob_id": 1234,
            "hub_id": "admin",
            "person": "JRFob",
            "time": 1574013874368,
            "loc": "office",
            "fob_state": "UNLOCKED",
            "_id": 22
        }
    ]
}
```


## List Authorized Fobs

Used to get a list of fobs that are authorized to access the system.


**URL** : `/fob-access/authorized/`

**Method** : `GET`

**Auth required** : YES

### Success Response

**Code** : `200 OK`

**Content example**

```json
{
    "fobs": [
        {
            "username": "User1",
            "fob_id": 1234,
            "_id": 2
        }
    ]
}
```


## Add an Authorized Fob

Used to add a new authorized fob to the system. Fobs must have unique IDs.

**URL** : `/fob-access/authorized`

**Method** : `POST`

**Auth required** : YES

**Example Body**

```json
{
	"username": "User1",
	"fob_id": 1234,
	"fob_code": 1234
}
```

### Success Response

**Code** : `204 No Content`



## Request Access

Used to request access for a specific fob id and code

**URL**: `/fob-acceess`

**Method**: `POST`

**Auth required**: YES

**Example Body**

```json
{
    "fob_id": 1234,
    "fob_code": 1234
}
```

### Success Response

**Code**: `200 OK`

**Content example**:

```json
{
    "accessRecord": {
        "fob_id": 1234,
        "hub_id": "admin",
        "person": "User1",
        "time": 1574388870261,
        "loc": "office",
        "_id": 26
    }
}
```
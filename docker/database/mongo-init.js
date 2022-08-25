db.createUser(
        {
                user: "defaultUser",
                pwd: "defaultPass",
                roles: [
                {
                        role: "readWrite",
                        db: "delay"
                }
                ]
        }
);

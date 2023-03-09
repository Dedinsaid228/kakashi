#include "include/packet/packet_mc.h"
#include "include/music_manager.h"
#include "include/packet/packet_factory.h"
#include "include/server.h"

#include <QDebug>

PacketMC::PacketMC(QStringList &contents) :
    AOPacket(contents)
{
}

PacketInfo PacketMC::getPacketInfo() const
{
    PacketInfo info{
        .acl_permission = ACLRole::Permission::NONE,
        .min_args = 2,
        .header = "MC"};
    return info;
}

void PacketMC::handlePacket(AreaData *area, AOClient &client) const
{
    // Due to historical reasons, this
    // packet has two functions:
    // Change area, and set music.

    // First, we check if the provided
    // argument is a valid song
    QString l_argument = m_content[0];

    if (client.getServer()->getMusicList().contains(l_argument) || client.m_music_manager->isCustom(client.m_current_area, l_argument) || l_argument == "~stop.mp3") { // ~stop.mp3 is a dummy track used by 2.9+
        // We have a song here

        if (client.m_is_spectator) {
            client.sendServerMessage("Spectator are blocked from changing the music.");
            return;
        }

        if (client.m_is_dj_blocked) {
            client.sendServerMessage("You are blocked from changing the music.");
            return;
        }

        if (!area->isMusicAllowed() && !client.checkPermission(ACLRole::CM)) {
            client.sendServerMessage("Music is disabled in this area.");
            return;
        }

        if (QDateTime::currentDateTime().toSecsSinceEpoch() - client.m_last_music_change_time <= 2) {
            client.sendServerMessage("You change music a lot!");
            return;
        }

        client.m_last_music_change_time = QDateTime::currentDateTime().toSecsSinceEpoch();

        QString l_effects;
        if (m_content.length() >= 4)
            l_effects = m_content[3];
        else
            l_effects = "0";
        QString l_final_song;

        // As categories can be used to stop music we need to check if it has a dot for the extension. If not, we assume its a category.
        if (!l_argument.contains("."))
            l_final_song = "~stop.mp3";
        else
            l_final_song = l_argument;

        AOPacket *l_music_change = PacketFactory::createPacket("MC", {l_final_song, m_content[1], client.m_showname, "1", "0", l_effects});
        client.getServer()->broadcast(l_music_change, client.m_current_area);

        // Since we can't ensure a user has their showname set, we check if its empty to prevent
        //"played by ." in /currentmusic.
        QString l_sender_name = client.getSenderName(client.m_id);

        area->changeMusic(l_sender_name, l_final_song);
        emit client.logMusic((client.m_current_char + " " + client.m_showname), client.m_ooc_name, client.m_ipid, client.getServer()->getAreaName(client.m_current_area), l_final_song, QString::number(client.m_id), client.m_hwid, client.getServer()->getHubName(client.m_hub));
        return;
    }

    for (int i = 0; i < client.getServer()->getAreaCount(); i++) {
        QString l_area = client.getServer()->getAreaName(i);
        if (l_area == l_argument) {
            client.changeArea(i);
            break;
        }
    }
}

bool PacketMC::validatePacket() const
{
    return true;
}

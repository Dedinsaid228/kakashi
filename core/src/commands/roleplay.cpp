//////////////////////////////////////////////////////////////////////////////////////
//    akashi - a server for Attorney Online 2                                       //
//    Copyright (C) 2020  scatterflower                                             //
//                                                                                  //
//    This program is free software: you can redistribute it and/or modify          //
//    it under the terms of the GNU Affero General Public License as                //
//    published by the Free Software Foundation, either version 3 of the            //
//    License, or (at your option) any later version.                               //
//                                                                                  //
//    This program is distributed in the hope that it will be useful,               //
//    but WITHOUT ANY WARRANTY; without even the implied warranty of                //
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                 //
//    GNU Affero General Public License for more details.                           //
//                                                                                  //
//    You should have received a copy of the GNU Affero General Public License      //
//    along with this program.  If not, see <https://www.gnu.org/licenses/>.        //
//////////////////////////////////////////////////////////////////////////////////////
#include "include/aoclient.h"

#include "include/area_data.h"
#include "include/config_manager.h"
#include "include/packet/packet_factory.h"
#include "include/server.h"

// This file is for commands under the roleplay category in aoclient.h
// Be sure to register the command in the header before adding it here!

void AOClient::cmdFlip(int argc, QStringList argv)
{
    Q_UNUSED(argc);
    Q_UNUSED(argv);

    QString l_sender_name = getSenderName(m_id);
    QStringList l_faces = {"heads", "tails"};
    QString l_face = l_faces[AOClient::genRand(0, 1)];

    sendServerMessageArea("[" + QString::number(m_id) + "] " + l_sender_name + " flipped a coin and got " + l_face + ".");
}

void AOClient::cmdRoll(int argc, QStringList argv)
{
    int l_sides = 6;
    int l_dice = 1;
    QStringList results;

    if (argc >= 1) {
        if (argv[0].contains('d')) {
            QStringList l_arguments = argv[0].split('d');

            bool l_dice_ok;
            bool l_sides_ok;
            l_dice = l_arguments[0].toInt(&l_dice_ok);
            l_sides = l_arguments[1].toInt(&l_sides_ok);

            if (argv[0].contains('+')) {
                bool l_mod_ok;
                QStringList l_modifier = l_arguments[1].split('+');
                int modifier = l_modifier[1].toInt(&l_mod_ok);
                l_sides = l_modifier[0].toInt(&l_sides_ok);

                if (l_mod_ok && l_dice_ok && l_sides_ok)
                    diceThrower(l_sides, l_dice, false, modifier);
                else
                    sendServerMessage("Invalid dice notation.");
                return;
            }
            else if (argv[0].contains('-')) {
                bool l_mod_ok;
                QStringList l_modifier = l_arguments[1].split('-');
                int modifier = l_modifier[1].toInt(&l_mod_ok);
                l_sides = l_modifier[0].toInt(&l_sides_ok);

                if (l_mod_ok && l_dice_ok && l_sides_ok)
                    diceThrower(l_sides, l_dice, false, -modifier);
                else
                    sendServerMessage("Invalid dice notation.");
                return;
            }
            else if (l_dice_ok && l_sides_ok) {
                diceThrower(l_sides, l_dice, false);
                return;
            }
            else {
                sendServerMessage("Invalid dice notation.");
                return;
            }
        }
        else
            l_sides = qBound(1, argv[0].toInt(), ConfigManager::diceMaxValue());
    }
    if (argc == 2)
        l_dice = qBound(1, argv[1].toInt(), ConfigManager::diceMaxDice());
    diceThrower(l_sides, l_dice, false);
}

void AOClient::cmdRollP(int argc, QStringList argv)
{
    int l_sides = 6;
    int l_dice = 1;
    QStringList results;

    if (argc >= 1) {
        if (argv[0].contains('d')) {
            QStringList l_arguments = argv[0].split('d');

            bool l_dice_ok;
            bool l_sides_ok;
            l_dice = l_arguments[0].toInt(&l_dice_ok);
            l_sides = l_arguments[1].toInt(&l_sides_ok);

            if (argv[0].contains('+')) {
                bool l_mod_ok;
                QStringList l_modifier = l_arguments[1].split('+');
                int modifier = l_modifier[1].toInt(&l_mod_ok);
                l_sides = l_modifier[0].toInt(&l_sides_ok);

                if (l_mod_ok && l_dice_ok && l_sides_ok)
                    diceThrower(l_sides, l_dice, true, modifier);
                else
                    sendServerMessage("Invalid dice notation.");
                return;
            }
            else if (argv[0].contains('-')) {
                bool l_mod_ok;
                QStringList l_modifier = l_arguments[1].split('-');
                int modifier = l_modifier[1].toInt(&l_mod_ok);
                l_sides = l_modifier[0].toInt(&l_sides_ok);

                if (l_mod_ok && l_dice_ok && l_sides_ok)
                    diceThrower(l_sides, l_dice, true, -modifier);
                else
                    sendServerMessage("Invalid dice notation.");
                return;
            }
            else if (l_dice_ok && l_sides_ok) {
                diceThrower(l_sides, l_dice, true);
                return;
            }
            else {
                sendServerMessage("Invalid dice notation.");
                return;
            }
        }
        else
            l_sides = qBound(1, argv[0].toInt(), ConfigManager::diceMaxValue());
    }
    if (argc == 2)
        l_dice = qBound(1, argv[1].toInt(), ConfigManager::diceMaxDice());
    diceThrower(l_sides, l_dice, true);
}

void AOClient::cmdTimer(int argc, QStringList argv)
{
    AreaData *l_area = server->getAreaById(m_current_area);

    // Called without arguments
    // Shows a brief of all timers
    if (argc == 0) {
        QStringList timers;
        timers.append("Currently active timers:");

        for (int i = 0; i <= 4; i++) {
            timers.append(getAreaTimer(l_area->index(), i));
        }

        sendServerMessage(timers.join("\n"));
        return;
    }

    // Called with more than one argument
    bool ok;
    int l_timer_id = argv[0].toInt(&ok);

    if (!ok || l_timer_id < 0 || l_timer_id > 4) {
        sendServerMessage("Invalid timer ID. Timer ID must be a whole number between 0 and 4.");
        return;
    }

    // Called with one argument
    // Shows the status of one timer
    if (argc == 1) {
        sendServerMessage(getAreaTimer(l_area->index(), l_timer_id));
        return;
    }

    // Called with more than one argument
    // Updates the state of a timer

    // Select the proper timer
    // Check against permissions if global timer is selected
    QTimer *l_requested_timer;
    if (l_timer_id == 0) {
        if (!checkPermission(ACLRole::GLOBAL_TIMER)) {
            sendServerMessage("You are not authorized to alter the global timer.");
            return;
        }

        l_requested_timer = server->timer;
    }
    else
        l_requested_timer = l_area->timers().at(l_timer_id - 1);

    AOPacket *l_show_timer = PacketFactory::createPacket("TI", {QString::number(l_timer_id), "2"});
    AOPacket *l_hide_timer = PacketFactory::createPacket("TI", {QString::number(l_timer_id), "3"});
    bool l_is_global = l_timer_id == 0;

    // Set the timer's time remaining if the second
    // argument is a valid time
    QTime l_requested_time = QTime::fromString(argv[1], "hh:mm:ss");

    if (l_requested_time.isValid()) {
        l_requested_timer->setInterval(QTime(0, 0).msecsTo(l_requested_time));
        l_requested_timer->start();
        sendServerMessage("Set timer " + QString::number(l_timer_id) + " to " + argv[1] + ".");
        AOPacket *l_update_timer = PacketFactory::createPacket("TI", {QString::number(l_timer_id), "0", QString::number(QTime(0, 0).msecsTo(l_requested_time))});
        l_is_global ? server->broadcast(l_show_timer) : server->broadcast(l_show_timer, m_current_area); // Show the timer
        l_is_global ? server->broadcast(l_update_timer) : server->broadcast(l_update_timer, m_current_area);
        return;
    }
    // Otherwise, update the state of the timer
    else {
        if (argv[1] == "start") {
            l_requested_timer->start();
            sendServerMessage("Started timer " + QString::number(l_timer_id) + ".");
            AOPacket *l_update_timer = PacketFactory::createPacket("TI", {QString::number(l_timer_id), "0", QString::number(QTime(0, 0).msecsTo(QTime(0, 0).addMSecs(l_requested_timer->remainingTime())))});
            l_is_global ? server->broadcast(l_show_timer) : server->broadcast(l_show_timer, m_current_area);
            l_is_global ? server->broadcast(l_update_timer) : server->broadcast(l_update_timer, m_current_area);
        }
        else if (argv[1] == "pause" || argv[1] == "stop") {
            l_requested_timer->setInterval(l_requested_timer->remainingTime());
            l_requested_timer->stop();
            sendServerMessage("Stopped timer " + QString::number(l_timer_id) + ".");
            AOPacket *l_update_timer = PacketFactory::createPacket("TI", {QString::number(l_timer_id), "1", QString::number(QTime(0, 0).msecsTo(QTime(0, 0).addMSecs(l_requested_timer->interval())))});
            l_is_global ? server->broadcast(l_update_timer) : server->broadcast(l_update_timer, m_current_area);
        }
        else if (argv[1] == "hide" || argv[1] == "unset") {
            l_requested_timer->setInterval(0);
            l_requested_timer->stop();
            sendServerMessage("Hid timer " + QString::number(l_timer_id) + ".");
            // Hide the timer
            l_is_global ? server->broadcast(l_hide_timer) : server->broadcast(l_hide_timer, m_current_area);
        }
    }
}

void AOClient::cmdNoteCard(int argc, QStringList argv)
{
    Q_UNUSED(argc);

    AreaData *l_area = server->getAreaById(m_current_area);
    QString l_notecard = argv.join(" ");
    QString l_sender_name = getSenderName(m_id);

    l_area->addNotecard(m_current_char, l_notecard);
    sendServerMessageArea("[" + QString::number(m_id) + "] " + l_sender_name + " wrote a note card.");
}

void AOClient::cmdNoteCardClear(int argc, QStringList argv)
{
    Q_UNUSED(argc);
    Q_UNUSED(argv);

    AreaData *l_area = server->getAreaById(m_current_area);
    QString l_sender_name = getSenderName(m_id);

    if (!l_area->addNotecard(m_current_char, QString()))
        sendServerMessageArea("[" + QString::number(m_id) + "] " + l_sender_name + " erased their note card.");
}

void AOClient::cmdNoteCardReveal(int argc, QStringList argv)
{
    Q_UNUSED(argc);
    Q_UNUSED(argv);

    AreaData *l_area = server->getAreaById(m_current_area);
    const QStringList l_notecards = l_area->getNotecards();

    if (l_notecards.isEmpty()) {
        sendServerMessage("There are no cards to reveal in this area.");
        return;
    }

    QString l_message("Note cards have been revealed.\n");
    l_message.append(l_notecards.join(""));

    sendServerMessageArea(l_message);
}

void AOClient::cmd8Ball(int argc, QStringList argv)
{
    Q_UNUSED(argc);

    if (ConfigManager::magic8BallAnswers().isEmpty()) {
        qWarning() << "8ball.txt is empty!";
        sendServerMessage("8ball.txt is empty.");
    }
    else {
        QString l_response = ConfigManager::magic8BallAnswers().at((genRand(1, ConfigManager::magic8BallAnswers().size() - 1)));
        QString l_sender_name = getSenderName(m_id);
        QString l_sender_id = "[" + QString::number(m_id) + "] ";
        QString l_sender_message = argv.join(" ");

        sendServerMessageArea(l_sender_id + l_sender_name + " asked the magic 8-ball, \"" + l_sender_message + "\" and the answer is: " + l_response);
    }
}

void AOClient::cmdSubTheme(int argc, QStringList argv)
{
    Q_UNUSED(argc);

    QString l_subtheme = argv.join(" ");

    const QVector<AOClient *> l_clients = server->getClients();
    for (AOClient *l_client : l_clients) {
        if (l_client->m_current_area == m_current_area)
            l_client->sendPacket("ST", {l_subtheme, "1"});
    }

    sendServerMessageArea("Subtheme was set to " + l_subtheme);
}

void AOClient::cmdVote(int argc, QStringList argv)
{
    AreaData *l_area = server->getAreaById(m_current_area);

    if (argc == 0 && checkPermission(ACLRole::CM)) {
        l_area->toggleVote();

        QString l_state = l_area->isVoteStarted() ? "started!" : "forcibly stopped!";
        sendServerMessageArea("Voting is " + l_state);

        if (l_area->isVoteStarted()) {
            QString l_candidates = "Candidates:";
            int l_candidates_count = 0;
            const QVector<AOClient *> l_clients = server->getClients();

            for (AOClient *l_client : l_clients) {
                if ((l_area->lockStatus() == AreaData::LockStatus::FREE || l_area->invited().contains(l_client->m_id)) && l_client->m_ipid != m_ipid && l_client->m_current_area == m_current_area) {
                    l_client->m_can_vote = true;
                    l_client->m_vote_candidate = true;
                    l_client->m_vote_points = 0;
                    l_candidates_count++;
                    l_candidates += "\n[" + QString::number(l_client->m_id) + "] " + getSenderName(l_client->m_id);
                }
            }

            if (l_candidates_count <= 2) {
                sendServerMessageArea("ERROR! Candidates were not found or there are few of them. Voting has been stopped. Please find more players.\n"
                                      "- CM that started voting does not count, as well as all its open clients.\n"
                                      "- If area is set to SPECTATABLE or LOCKED, then only invited clients can be candidates.");
                l_area->toggleVote();
                return;
            }

            sendServerMessageArea(l_candidates);
        }
        else
            endVote();
    }
    else if (argc > 0 && m_can_vote) {
        if (argc > 1) {
            sendServerMessage("Too many arguments!");
            return;
        }

        bool l_ok;
        AOClient *l_target_client = server->getClientByID(argv[0].toInt(&l_ok));

        if (!l_ok) {
            sendServerMessage("That doesn't look like a valid ID.");
            return;
        }

        if (l_target_client == nullptr) {
            sendServerMessage("Unable to find client with ID " + argv[0] + ".");
            return;
        }

        if (!l_target_client->m_vote_candidate) {
            sendServerMessage("This client is not a candidate!");
            return;
        }

        sendServerMessageArea("[" + QString::number(m_id) + "] " + getSenderName(m_id) + " voted.");
        l_target_client->m_vote_points++;

        m_can_vote = false;
        bool l_all_voted = true;
        const QVector<AOClient *> l_clients = server->getClients();

        for (AOClient *l_client : l_clients) {
            if (l_client->m_current_area == m_current_area && l_client->m_can_vote)
                l_all_voted = false;
        }

        if (l_all_voted) {
            endVote();
            l_area->toggleVote();
        }
    }
    else
        sendServerMessage("You didn't provide an ID, you have already voted, or you are not allowed to use the command.");
}
